#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <ctime>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/int32.hpp"
#include "sensor_msgs/msg/joy.hpp"

using namespace std::chrono_literals;

/* This example creates a subclass of Node and uses std::bind() to register a
* member function as a callback from the timer. */

// Button number
const int IDLE_BTN_NUM = 3;
const int FOLLOW_BTN_NUM = 1;
const int TELEOP_BTN_NUM = 0;
// Flag
bool idle_flag = false;
bool follow_flag = false;
bool teleop_flag = false;
// State
int robot_state = 0;
int last_robot_state = 5;
bool target_state = false;
// Timer
time_t time0=0;
time_t time1=0;


class RobotController : public rclcpp::Node
{
  public:
    RobotController():Node("robot_controller")
    {
        // Create Publisher
        idle_bool_pub_ = this->create_publisher<std_msgs::msg::Bool>("idle_bool", 10);
        robot_mode_pub_ = this->create_publisher<std_msgs::msg::String>("robot_mode", 10);
        led_state_pub_ = this->create_publisher<std_msgs::msg::Int32>("led_state", 10);
        // Create Subscriber
        joy_sub_ = this->create_subscription<sensor_msgs::msg::Joy>(
            "joy", 10, std::bind(&RobotController::button_callback, this, std::placeholders::_1));
        target_status_sub_ = this->create_subscription<std_msgs::msg::Bool>(
            "target_status", 10, std::bind(&RobotController::target_status_callback, this, std::placeholders::_1));
    }

  private:
    // Publisher
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr idle_bool_pub_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr robot_mode_pub_;
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr led_state_pub_;
    // Subscriber
    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr target_status_sub_;

    void button_callback(const sensor_msgs::msg::Joy::SharedPtr joy_msgs) const
    {
        // Idle Button
        if(joy_msgs->buttons[IDLE_BTN_NUM]==1 && idle_flag==false){
            idle_flag = true;
        }else{
            idle_flag = false;
        }
        // Follow Button
        if(joy_msgs->buttons[FOLLOW_BTN_NUM]==1 && follow_flag==false){
            follow_flag = true;
        }else{
            follow_flag = false;
        }
        // Teleop Button
        if(joy_msgs->buttons[TELEOP_BTN_NUM]==1 && teleop_flag==false){
            teleop_flag = true;
        }else{
            teleop_flag = false;
        }
        // messages
        auto idle_msgs = std_msgs::msg::Bool();
        auto mode_msgs = std_msgs::msg::String();
        auto led_msgs = std_msgs::msg::Int32();

        // FSM
        switch (robot_state){
            // Idle Mode
            case 0:
                mode_msgs.data = "IDLE";
                idle_msgs.data = true;
                led_msgs.data = 0;
                // Switch to Follow mode
                if(follow_flag==true && target_state==true) robot_state = 1;
                // Switch to Teleop mode
                if(teleop_flag==true) robot_state = 2;
                break;
            // Follow Mode
            case 1:
                mode_msgs.data = "FOLLOW";
                led_msgs.data = 1;
                idle_msgs.data = false;
                // Lose target for 5 sec
                if(target_state==true){
                    time0 = time(NULL);
                }else{
                    time1 = time(NULL);
                    if(time1>=(time0+5)) robot_state = 0;
                }
                if(idle_flag==true) robot_state = 0;
                break;
            // Teleop Mode
            case 2:
                mode_msgs.data = "TELEOP";
                led_msgs.data = 2;
                idle_msgs.data = false;
                if(idle_flag==true) robot_state = 0; 
                break;
            default:
                break;
        }

        // Publish data
        robot_mode_pub_->publish(mode_msgs);
        idle_bool_pub_->publish(idle_msgs);
        led_state_pub_->publish(led_msgs);
        
    }

    void target_status_callback(const std_msgs::msg::Bool::SharedPtr target_msgs) const
    {
    //   auto out_message = geometry_msgs::msg::Vector3();
      // Get the cmd vel
      // Publish data
    //   wheel_cmd_vel_pub_->publish(out_message);
    }

};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<RobotController>());
  rclcpp::shutdown();
  return 0;
}