# PlanSys2 Monitoring Example

## Description

This package is an extension of the [PlanSys2 Simple Example](https://github.com/PlanSys2/ros2_planning_system_examples/tree/rolling/plansys2_simple_example) and is meant to demonstrate a **monitoring extension for PlanSys2 plan execution**.

The package contains **predicate sensing plugins** examples that are loaded by PlanSys2 at runtime to verify whether action effects are actually true during execution, instead of being implicitly assumed.

The goal of this package is not to provide realistic sensing, but to demonstrate **how sensing plugins are integrated into PlanSys2** and can be used for monitoring.

The **PlanSys2 monitoring extension itself** is implemented in a fork of the official repository, in the `effect-monitoring` branch:
- https://github.com/eugenio24/ros2_planning_system/tree/effect-monitoring

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

## Fake Sensing Demo

This package includes also a **demo-only** class, `FakeSensingSequences`, located in: `src/fake_sensing_logic/`. It is provided purely for demo/testing purposes. It is **not required** for the actual sensing plugin integration.

It simulates sensing results using a predefined sequence, configurable via a YAML file: `src/fake_sensing_logic/fake_sensing_config.yaml`

Example configuration:

```yaml
sequence_file: "/plansys2_dev/plansys2_ws/src/plansys2_monitoring_example/src/fake_sensing_logic/util/sequence.txt"

random_failure:
  enabled: true
  probability: 0.15
  seed: 1

loop_sequence: false
```

* Each sensing query consumes the next value in the sequence and returns the corresponding boolean.
* Random failures can be enabled using `random_failure`, with a reproducible `seed`.
* `loop_sequence` controls whether the sequence restarts when it reaches the end.
* Execution is deterministic and reproducible. This allows testing and demonstrating monitoring behavior **without real sensors**.

### Generating the Sequence File

The sequence file (`sequence.txt`) can be automatically generated from a **PDDL domain** and a **plan file** using the provided Python script: `src/fake_sensing_logic/util/generate_sequence.py`

This small utility is not a complete PDDL parser but is intended to facilitate debugging and testing. It generates a simple, flat list of effects to be sensed.


Usage:
```bash
python generate_sequence.py domain.pddl plan.txt sequence.txt
```

* **domain.pddl**, the PDDL domain file defining actions and effects. Example: `/pddl/simple_example.pddl`.
* **plan.txt**, a plan file containing the output of the plan that will be executed (`get plan` command in `plansys2_terminal`). Example: `/src/fake_sensing_logic/util/plan.txt`.
* **sequence.txt**, the generated sequence file to be used by `FakeSensingSequences`.

The generated sequence file will look like:
```
predicate arg1 arg2 ... true|false
```

This allows `FakeSensingSequences` to **replay the expected effects deterministically**, optionally with random failures.
