// Copyright 2025 Intelligent Robotics Lab
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

#include <string>
#include <vector>
#include <memory>
#include <rclcpp/rclcpp.hpp>

#include "plansys2_executor/PredicateSensingBase.hpp"

#include "FakeSensingSequences.hpp"

using plansys2::PredicateSensingBase;
using plansys2::SensingResult;

namespace plansys2_sensing_plugins
{

class RobotAtPredicateSensor : public PredicateSensingBase
{
public:
    RobotAtPredicateSensor()
    {
        predicate_name_ = "robot_at";
    }

    SensingResult sense(const std::vector<std::string>& args) override
    {
        if (args.size() != 2) {
            return SensingResult{SensingResult::ERROR, "robot_at expects 2 arguments: (robot, room)"};
        }

        const std::string& robot = args[0];
        const std::string& room  = args[1];

        return sense(robot, room);
    }

private:
    SensingResult sense(const std::string& robot, const std::string& room)
    {
        // Example sensing logic (replace with real localization/TF)
        RCLCPP_INFO(rclcpp::get_logger("RobotAtPredicateSensor"),
                    "Checking if robot '%s' is in room '%s'",
                    robot.c_str(), room.c_str());

        bool detected = FakeSensingSequences::getInstance()->get(predicate_name_, {robot, room});

        if (detected) {
            return SensingResult{SensingResult::TRUE, robot + " is at " + room};
        } else {
            return SensingResult{SensingResult::FALSE, robot + " is NOT at " + room};
        }
    }
};

}   // namespace plansys2_sensing_plugins

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(
    plansys2_sensing_plugins::RobotAtPredicateSensor, 
    plansys2::PredicateSensingBase
)
