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

#include "plansys2_executor/effect_monitoring/plugin_interfaces/PredicateSensingBase.hpp"

#include "fake_sensing_logic/FakeSensingSequences.hpp"

using plansys2::PredicateSensingBase;
using plansys2::PredicateSensingResult;

namespace plansys2_sensing_plugins
{

class CarAtPredicateSensor : public PredicateSensingBase
{
public:
  CarAtPredicateSensor()
  {
    symbol_name_ = "car_at";
  }

  PredicateSensingResult sense(const std::vector<std::string>& args) override
  {
    if (args.size() != 2) {
      return PredicateSensingResult{
        PredicateSensingResult::Status::ERROR,
        "cat_at expects 2 argument: (car, waypoint)"
      };
    }

    const std::string & car = args[0];
    const std::string & waypoint = args[1];
    return sense(car, waypoint);
  }

private:
  PredicateSensingResult sense(const std::string & car, const std::string & waypoint)
  {
    RCLCPP_INFO(rclcpp::get_logger("CarAtPredicateSensor"),
      "Checking if car '%s' is at waypoint '%s'", car.c_str(), waypoint.c_str());

    bool result = FakeSensingSequences::getInstance()->sensePredicate(symbol_name_, {car, waypoint});

    if (result) {
      return PredicateSensingResult{PredicateSensingResult::Status::TRUE, car + " is at " + waypoint};
    } else {
      return PredicateSensingResult{PredicateSensingResult::Status::FALSE, car + " is not at " + waypoint};
    }
  }
};

} // namespace plansys2_sensing_plugins

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(
  plansys2_sensing_plugins::CarAtPredicateSensor,
  plansys2::PredicateSensingBase
)
