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

class BatteryFullPredicateSensor : public PredicateSensingBase
{
public:
    BatteryFullPredicateSensor()
    {
        predicate_name_ = "battery_full";
    }

    SensingResult sense(const std::vector<std::string>& args) override
    {
        if (args.size() != 1) {
            return SensingResult{
                SensingResult::ERROR,
                "battery_full expects 1 argument: (robot)"
            };
        }

        const std::string & robot = args[0];
        return sense(robot);
    }

private:
    SensingResult sense(const std::string & robot)
    {
        RCLCPP_INFO(rclcpp::get_logger("BatteryFullPredicateSensor"),
                    "Checking if robot '%s' has FULL battery", robot.c_str());

        bool is_full = FakeSensingSequences::getInstance()->get(predicate_name_, {robot});

        if (is_full) {
            return SensingResult{SensingResult::TRUE, robot + " battery is FULL"};
        } else {
            return SensingResult{SensingResult::FALSE, robot + " battery is NOT full"};
        }
    }
};

} // namespace plansys2_sensing_plugins

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(
    plansys2_sensing_plugins::BatteryFullPredicateSensor,
    plansys2::PredicateSensingBase
)
