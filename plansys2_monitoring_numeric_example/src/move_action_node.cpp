// Copyright 2019 Intelligent Robotics Lab
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <memory>
#include <algorithm>

#include "plansys2_executor/ActionExecutorClient.hpp"
#include "plansys2_problem_expert/ProblemExpertClient.hpp"

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

using namespace std::chrono_literals;

class Move : public plansys2::ActionExecutorClient
{
public:
  Move()
  : plansys2::ActionExecutorClient("move", 500ms),
    progress_(0.0),
    duration_(-1.0)
  {
  }

private:
  void do_work()
  {
    // compute duration on first tick
    if (duration_ < 0.0) {
      auto args = get_arguments();
      // args[0] = car, args[1] = wp1, args[2] = wp2
      std::string car = args[0];
      std::string wp1 = args[1];
      std::string wp2 = args[2];

      auto problem_client = std::make_shared<plansys2::ProblemExpertClient>();

      auto distance = problem_client->getFunction("distance(" + wp1 + "," + wp2 + ")");
      auto speed    = problem_client->getFunction("speed(" + car + ")");

      if (!distance.has_value() || !speed.has_value() || speed->value == 0.0) {
        RCLCPP_ERROR(get_logger(), "Could not compute move duration");
        finish(false, 0.0, "Move failed: missing functions");
        return;
      }

      duration_ = distance->value / speed->value;
      RCLCPP_INFO(get_logger(), "Move duration computed: %.2f seconds", duration_);
    }

    // tick rate is 500ms = 0.5s, so increment = 0.5 / duration
    progress_ += 0.5 / duration_;
    progress_ = std::min(progress_, 1.0f);

    if (progress_ >= 1.0) {
      finish(true, 1.0, "Move completed");

      progress_ = 0.0;
      duration_ = -1.0;
      std::cout << std::endl;
    } else {
      send_feedback(progress_, "Move running");
    }

    std::cout << "\r\e[K" << std::flush;
    std::cout << "Moving ... [" << std::min(100.0, progress_ * 100.0) << "%]  " 
              << std::flush;
  }

  float progress_;
  double duration_;
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<Move>();

  node->set_parameter(rclcpp::Parameter("action_name", "move"));
  node->trigger_transition(lifecycle_msgs::msg::Transition::TRANSITION_CONFIGURE);
  
  rclcpp::spin(node->get_node_base_interface());
  
  rclcpp::shutdown();
  
  return 0;
}
