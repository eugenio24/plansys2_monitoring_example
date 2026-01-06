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

class VisitedPredicateSensor : public PredicateSensingBase
{
public:
  VisitedPredicateSensor()
  {
    symbol_name_ = "visited";
  }

  PredicateSensingResult sense(const std::vector<std::string>& args) override
  {
    if (args.size() != 1) {
      return PredicateSensingResult{
        PredicateSensingResult::Status::ERROR,
        "visited expects 1 argument: (waypoint)"
      };
    }

    const std::string & waypoint = args[0];
    return sense(waypoint);
  }

private:
  PredicateSensingResult sense(const std::string & waypoint)
  {
    RCLCPP_INFO(rclcpp::get_logger("VisitedPredicateSensor"),
      "Checking if waypoint '%s' has been visted", waypoint.c_str());

    bool visited = FakeSensingSequences::getInstance()->sensePredicate(symbol_name_, {waypoint});

    if (visited) {
      return PredicateSensingResult{PredicateSensingResult::Status::TRUE, waypoint + " has been visited"};
    } else {
      return PredicateSensingResult{PredicateSensingResult::Status::FALSE, waypoint + " has not been visited"};
    }
  }
};

} // namespace plansys2_sensing_plugins

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(
  plansys2_sensing_plugins::VisitedPredicateSensor,
  plansys2::PredicateSensingBase
)
