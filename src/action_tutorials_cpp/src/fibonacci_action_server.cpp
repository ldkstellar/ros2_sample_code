#include <functional>
#include <memory>
#include <thread>

#include "action_tutorials_interfaces/action/fibonacci.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"

namespace action_tutorials_cpp
{
    class FibonacciActionServer : public rclcpp::Node
    {
    public:
        using Fibonacci = action_tutorials_interfaces::action::Fibonacci;       // 타입 축약
        using GoalHandleFibonacci = rclcpp_action::ServerGoalHandle<Fibonacci>; // 클래스를 축약

        explicit FibonacciActionServer(const rclcpp::NodeOptions &options = rclcpp::NodeOptions()) // 인자를 안 받았을 때 사용할 기본값
            : Node("fibonacci_action_server", options)                                             // 노드 생성
        {
            using namespace std::placeholders;

            this->action_server_ = rclcpp_action::create_server<Fibonacci>( // 서버생성
                this,
                "fibonacci", // action 이름
                std::bind(&FibonacciActionServer::handle_goal, this, _1, _2),
                std::bind(&FibonacciActionServer::handle_cancel, this, _1),
                std::bind(&FibonacciActionServer::handle_accepted, this, _1));
        }

    private:
        rclcpp_action::Server<Fibonacci>::SharedPtr action_server_; // 액션 서버 공유 포인터

        rclcpp_action::GoalResponse handle_goal( //
            const rclcpp_action::GoalUUID &uuid,
            std::shared_ptr<const Fibonacci::Goal> goal)
        {
            RCLCPP_INFO(this->get_logger(), "Received goal request with order %d", goal->order);
            (void)uuid; // UUID는 목표 요청을 식별하기 위한 고유 식별자야.

            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE; // 리턴값으로 목표 수락, 거절하는 것은 리턴값에 따라 다르다.
        } // 이 함수는 클라이언트가 목표를 요청했을 때 호출돼.

        rclcpp_action::CancelResponse handle_cancel( // 목표 거절
            const std::shared_ptr<GoalHandleFibonacci> goal_handle)
        {
            RCLCPP_INFO(this->get_logger(), "Received request to cancel goal");
            (void)goal_handle;
            return rclcpp_action::CancelResponse::ACCEPT;
        }

        void handle_accepted(const std::shared_ptr<GoalHandleFibonacci> goal_handle) // 목표 수락
        {
            using namespace std::placeholders;
            // this needs to return quickly to avoid blocking the executor, so spin up a new thread
            std::thread{std::bind(&FibonacciActionServer::execute, this, _1), goal_handle}.detach(); // 스레드를 현재 스레드와 분리해 독립적으로 실행하도록 해.
        }

        void execute(const std::shared_ptr<GoalHandleFibonacci> goal_handle)
        {
            RCLCPP_INFO(this->get_logger(), "Executing goal");
            rclcpp::Rate loop_rate(1);                               // 반복문 주기를 1초
            const auto goal = goal_handle->get_goal();               // 목표 정보를 가져온다
            auto feedback = std::make_shared<Fibonacci::Feedback>(); // 피드백 객체를 동적으로 가져온다
            auto &sequence = feedback->partial_sequence;             // 참조로 가져온다.
            // 초기 값
            sequence.push_back(0);
            sequence.push_back(1);
            auto result = std::make_shared<Fibonacci::Result>(); // 결과 객체

            for (int i = 1; (i < goal->order) && rclcpp::ok(); ++i)
            {
                // Check if there is a cancel request
                if (goal_handle->is_canceling())
                {
                    result->sequence = sequence;
                    goal_handle->canceled(result);
                    RCLCPP_INFO(this->get_logger(), "Goal canceled");
                    return;
                }
                // Update sequence
                sequence.push_back(sequence[i] + sequence[i - 1]);
                // Publish feedback
                goal_handle->publish_feedback(feedback); // 피드백 전달
                RCLCPP_INFO(this->get_logger(), "Publish feedback");

                loop_rate.sleep();
            }

            // Check if goal is done
            if (rclcpp::ok())
            {
                result->sequence = sequence;  // 값을 복사
                goal_handle->succeed(result); // 결과 값 전달
                RCLCPP_INFO(this->get_logger(), "Goal succeeded");
            }
        }
    }; // class FibonacciActionServer

} // namespace action_tutorials_cpp

RCLCPP_COMPONENTS_REGISTER_NODE(action_tutorials_cpp::FibonacciActionServer)