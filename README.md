# PlanSys2 Monitoring Example

## Description

This package is an extension of the [PlanSys2 Simple Example](https://github.com/PlanSys2/ros2_planning_system_examples/tree/rolling/plansys2_simple_example) and is meant to demonstrate a **monitoring extension for PlanSys2 plan execution**.

The package contains **predicate sensing plugins** examples that are loaded by PlanSys2 at runtime to verify whether action effects are actually true during execution, instead of being implicitly assumed.

The goal of this package is not to provide realistic sensing, but to demonstrate **how sensing plugins are integrated into PlanSys2** and can be used for monitoring.

The **PlanSys2 monitoring extension itself** is implemented in a fork of the official repository, in the `jazzy-devel` branch:
- https://github.com/eugenio24/ros2_planning_system/tree/jazzy-devel

---

## Sensing Plugins Overview

Each plugin:
- Implements `plansys2::PredicateSensingBase`
- Is responsible for sensing the truth value of a specific predicate
- Is loaded dynamically by PlanSys2 via `pluginlib`
- the plugins must be implemented in the namespace: `plansys2_sensing_plugins`

Example plugin implementations can be found in:
```
src/sensing_plugins/
```

---

## Enabling Monitoring in PlanSys2

To enable the PlanSys2 **monitoring extension** during plan execution, specify the monitored action bt (`plansys2_action_bt_monitored.xml`) via the `action_bt_file` argument in your Python launch file. As an example, see: `plansys2_ws/src/plansys2_monitoring_example/launch/plansys2_monitoring_example_launch.py`.

```python
'action_bt_file': os.path.join(
    get_package_share_directory('plansys2_executor'),
    'behavior_trees',
    'plansys2_action_bt_monitored.xml')
```

This tells the PlanSys2 behavior tree builder to use, for each action, a subtree that includes additional nodes to check the action's start and end effects, instead of the default subtree.

---

## Registering Sensing Plugins with PlanSys2

### 1. CMake Configuration

Add to `CMakeLists.txt` the sensing plugins so that they are compiled as a shared library:

```cmake
add_library(
  plansys2_sensing_plugins
  SHARED
  src/sensing_plugins/battery_full_sensing.cpp
  src/sensing_plugins/battery_low_sensing.cpp
  src/sensing_plugins/robot_at_sensing.cpp
)
````

```cmake
ament_target_dependencies(plansys2_sensing_plugins
  rclcpp
  plansys2_executor
  pluginlib
)
```

---

### 2. Plugin Description File

Plugins are exported via a `pluginlib` XML file: `plansys2_monitoring_example_sensing_plugins.xml`

```xml
<library path="plansys2_sensing_plugins">

  <class
    type="plansys2_sensing_plugins::RobotAtPredicateSensor"
    base_class_type="plansys2::PredicateSensingBase">
    <description>RobotAt sensing plugin.</description>
  </class>

  <class
    type="plansys2_sensing_plugins::BatteryFullPredicateSensor"
    base_class_type="plansys2::PredicateSensingBase">
    <description>BatteryFull sensing plugin.</description>
  </class>

  <class
    type="plansys2_sensing_plugins::BatteryLowPredicateSensor"
    base_class_type="plansys2::PredicateSensingBase">
    <description>BatteryLow sensing plugin.</description>
  </class>

</library>
```

The namespace **must match exactly** the plugin implementation:

```cpp
plansys2_sensing_plugins::*
```

---

### 3. Exporting Plugins

The plugin description is exported in `CMakeLists.txt` using:

```cmake
pluginlib_export_plugin_description_file(
  plansys2_executor
  plansys2_monitoring_example_sensing_plugins.xml
)

ament_export_libraries(plansys2_sensing_plugins)
```

The XML file is installed so that PlanSys2 can discover it:

```cmake
install(FILES
  plansys2_monitoring_example_sensing_plugins.xml
  DESTINATION share/${PROJECT_NAME}
)
```

---

### 4. Exporting the Plugin Class with `pluginlib`

At the bottom of each plugin implementation file, add the following:
```cpp
#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(
    plansys2_sensing_plugins::RobotAtPredicateSensor, 
    plansys2::PredicateSensingBase
)
```

---

For more information about how `pluginlib` works see the official ROS 2 documentation:
> [https://docs.ros.org/en/foxy/Tutorials/Beginner-Client-Libraries/Pluginlib.html](https://docs.ros.org/en/foxy/Tutorials/Beginner-Client-Libraries/Pluginlib.html)

---

## Fake Sensing Example

This package also includes a `FakeSensingSequences` class, located in:

```
src/fake_sensing_logic/
```

It **simulates sensing results** using a predefined sequence stored in:

```
src/fake_sensing_logic/sequence.txt
```

Each sensing query consumes the next value in the sequence and returns the corresponding boolean result. This allows deterministic and reproducible execution runs, making it easier to demonstrate and test the monitoring behavior without relying on real perception.

---
