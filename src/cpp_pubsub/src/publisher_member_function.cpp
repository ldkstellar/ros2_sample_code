#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

/* This example creates a subclass of Node and uses std::bind() to register a
 * member function as a callback from the timer. */

class MinimalPublisher : public rclcpp::Node
{
public:
    MinimalPublisher()
        : Node("minimal_publisher"), count_(0) // 초기화 리스트 부모클래스 생성자 호출(노드 이름) 및 변수 초기화
    {
        publisher_ = this->create_publisher<std_msgs::msg::String>("topic", 10); // 퍼플리셔 생성(토픽이름 큐사이즈)
        timer_ = this->create_wall_timer(
            500ms, std::bind(&MinimalPublisher::timer_callback, this)); // 타이머 생성 메서드(초 ,콜백함수)
    }

private:
    void timer_callback()
    {
        auto message = std_msgs::msg::String();
        message.data = "Hello, world! " + std::to_string(count_++);
        RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", message.data.c_str());
        publisher_->publish(message);
    }
    rclcpp::TimerBase::SharedPtr timer_;                            // 타이머 공유 포인터
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_; // 퍼블리셔 공유 포인터
    size_t count_;                                                  // 카운트 변수
};

int main(int argc, char *argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MinimalPublisher>());
    rclcpp::shutdown();
    return 0;
}