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

#include "plansys2_executor/effect_monitoring/plugin_interfaces/FunctionSensingBase.hpp"

#include "fake_sensing_logic/FakeSensingSequences.hpp"

using plansys2::FunctionSensingBase;
using plansys2::FunctionSensingResult;

namespace plansys2_sensing_plugins
{

class FuelLevelFunctionSensor : public FunctionSensingBase
{
public:
  FuelLevelFunctionSensor()
  {
    symbol_name_ = "fuel_level";
  }

  FunctionSensingResult sense(const std::vector<std::string>& args) override
  {
    if (args.size() != 1) {
      return FunctionSensingResult{
        FunctionSensingResult::Status::ERROR,
        std::nullopt,
        "fuel_level expects 1 argument: (car)"
      };
    }

    const std::string & car = args[0];
    return sense(car);
  }

private:
  FunctionSensingResult sense(const std::string & car)
  {
    RCLCPP_INFO(rclcpp::get_logger("FuelLevelFunctionSensor"),
      "Checking fuel level for car '%s'", car.c_str());

    auto fuel_level = FakeSensingSequences::getInstance()->senseFunction(symbol_name_, {car});

    if (fuel_level.has_value()) {
      return FunctionSensingResult{FunctionSensingResult::Status::OK, fuel_level.value()};
    } else {
      return FunctionSensingResult{FunctionSensingResult::Status::ERROR, std::nullopt, "Cannot retrieve fuel_level for car: " + car};
    }
  }
};

} // namespace plansys2_sensing_plugins

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(
  plansys2_sensing_plugins::FuelLevelFunctionSensor,
  plansys2::FunctionSensingBase
)
