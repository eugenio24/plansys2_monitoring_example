cmake_minimum_required(VERSION 3.5)
project(plansys2_monitoring_numeric_example)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(plansys2_msgs REQUIRED)
find_package(plansys2_executor REQUIRED)
find_package(pluginlib REQUIRED)

# Demo-only: FakeSensingSequences support
# This is not required for the real sensing plugin integration.
# Only needed by the demo fakesensing class.
find_package(plansys2_fake_sensing_utils REQUIRED)

set(CMAKE_CXX_STANDARD 17)

set(dependencies
  rclcpp
  plansys2_msgs
  plansys2_executor
)

add_executable(move_action_node src/move_action_node.cpp)
ament_target_dependencies(move_action_node ${dependencies})

add_executable(visit_action_node src/visit_action_node.cpp)
ament_target_dependencies(visit_action_node ${dependencies})

add_executable(refuel_action_node src/refuel_action_node.cpp)
ament_target_dependencies(refuel_action_node ${dependencies})

add_library(
  plansys2_sensing_plugins
  SHARED 
  src/sensing_plugins/car_at_sensing.cpp
  src/sensing_plugins/visited_sensing.cpp
  src/sensing_plugins/fuel_level_sensing.cpp
)

ament_target_dependencies(plansys2_sensing_plugins
  rclcpp
  plansys2_executor
  pluginlib
  plansys2_fake_sensing_utils   # only for the fake_sensing_utils
)

pluginlib_export_plugin_description_file(plansys2_executor plansys2_monitoring_numeric_example_sensing_plugins.xml)

install(DIRECTORY launch pddl DESTINATION share/${PROJECT_NAME})

install(FILES
  plansys2_monitoring_numeric_example_sensing_plugins.xml
  DESTINATION share/${PROJECT_NAME}
)

install(TARGETS
  move_action_node
  visit_action_node
  refuel_action_node

  plansys2_sensing_plugins
  
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_gtest REQUIRED)
endif()

ament_export_libraries(plansys2_sensing_plugins)
ament_export_dependencies(${dependencies})

ament_package()