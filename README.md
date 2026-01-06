# PlanSys2 Monitoring Examples

This repository provides example packages demonstrating a **monitoring extension for PlanSys2 plan execution**.

The **PlanSys2 monitoring extension itself** is implemented in a fork of the official repository, in the `effect-monitoring` branch:
- https://github.com/eugenio24/ros2_planning_system/tree/effect-monitoring

TODO add a section that explains how the monitoring is done

---

### Example Packages
- `plansys2_monitoring_simple_example`: A simple example based on the [PlanSys2 Simple Example](https://github.com/PlanSys2/ros2_planning_system_examples/tree/rolling/plansys2_simple_example)

- `plansys2_monitoring_numeric_example`: An example that also includes numeric fluents and numeric expressions in the effects.

The goal of these examples is not to provide realistic sensing, but to demonstrate **how sensing plugins are integrated into PlanSys2** and can be used for monitoring.

---

## Sensing Plugins Overview

The monitoring is performed with sensing plugins, which implement one of two interfaces:

| Effect Type           | Interface                        |
| --------------------- | -------------------------------- |
| Predicate effect      | `plansys2::PredicateSensingBase` |
| Functions/Numeric expression effect        | `plansys2::FunctionSensingBase`  |


Each plugin:
- Implement the corresponding interface
- Sense either the truth value of a predicate or the value of a function.
- Is loaded dynamically by PlanSys2 via `pluginlib`
- Must be implemented in the namespace: `plansys2_sensing_plugins`

Example plugin implementations can be found in:
```
plansys2_monitoring_simple_example/src/sensing_plugins
plansys2_monitoring_numeric_example/src/sensing_plugins
```

---

## Enabling Monitoring in PlanSys2

To enable the PlanSys2 **monitoring extension** during plan execution, specify the monitored action BT (`plansys2_action_bt_monitored.xml`) via the `action_bt_file` argument in your Python launch file. As an example, see: `plansys2_monitoring_simple_example/launch/plansys2_monitoring_simple_example_launch.py`.

```python
'action_bt_file': os.path.join(
    get_package_share_directory('plansys2_executor'),
    'behavior_trees',
    'plansys2_action_bt_monitored.xml'
)
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

Create a plugin XML file (e.g. `plansys2_monitoring_simple_example_sensing_plugins.xml`) to register your plugins with `pluginlib`

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

The plugins description is exported in `CMakeLists.txt` using:

```cmake
pluginlib_export_plugin_description_file(
  plansys2_executor
  plansys2_monitoring_simple_example_sensing_plugins.xml
)

ament_export_libraries(plansys2_sensing_plugins)
```

The XML file is installed so that PlanSys2 can discover it:

```cmake
install(FILES
  plansys2_monitoring_simple_example_sensing_plugins.xml
  DESTINATION share/${PROJECT_NAME}
)
```

---

### 4. Exporting the Plugin Class

At the bottom of each plugin implementation, add the following:
```cpp
#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(
  plansys2_sensing_plugins::RobotAtPredicateSensor, 
  plansys2::PredicateSensingBase
)
```

---

Change `plansys2::PredicateSensingBase` with `plansys2::NumericSensingBase` for function plugins (see `plansys2_monitoring_numeric_example`).

---

For more information on how `pluginlib` works see the official ROS 2 documentation:
> [https://docs.ros.org/en/foxy/Tutorials/Beginner-Client-Libraries/Pluginlib.html](https://docs.ros.org/en/foxy/Tutorials/Beginner-Client-Libraries/Pluginlib.html)

---

## Fake Sensing Utility

This repository also includes a **demo-only** package `plansys2_fake_sensing_utils`.

This package is purely for demonstration and testing purposes and is **not required** for the actual sensing plugin integration.

It simulates sensing results using predefined sequences, configurable via a YAML file: `plansys2_fake_sensing_utils/config/fake_sensing_config.yaml`

Example configuration:

```yaml
predicates_sequence_file: /plansys2_dev/plansys2_ws/src/plansys2_monitoring_example/plansys2_fake_sensing_utils/util/simple-domain/predicates-sequence.txt
functions_sequence_file: null

random_failure:
  enabled: false
  probability: 0.15
  seed: 1

loop_sequence: false
```

Parameters:
* `predicates_sequence_file` / `functions_sequence_file` are the paths to the fake sensing sequences for predicates and functions.
* Each sensing query consumes the next value in the sequence and returns:
  * a boolean for predicates
  * a double for functions
* Random failures for predicates can be enabled using `random_failure`, with a reproducible `seed`.
* `loop_sequence` controls whether the sequence restarts when it reaches the end.

Execution is deterministic and reproducible. This allows testing and demonstrating monitoring behavior **without real sensors**.

### Generating the Sequence File
Sequence files can be automatically generated from:

1. A **PDDL domain**
2. A **plan file**
3. A **problem instance** (for function evaluation)

using the provided Python script:

```
plansys2_fake_sensing_utils/util/generate_sequence.py
```

This script has only been tested with the example domain and problem files in this repository, because it was implemented purely to avoid manually evaluating plans to extract their effects. Its purpose is solely to facilitate debugging and testing of the monitoring examples. It may contain bugs and is not intended as a general-purpose PDDL parser.

#### Usage

```bash
python generate_sequence.py domain.pddl plan.txt commands predicates.txt functions.txt
```

**Arguments:**

1. `domain.pddl`: the PDDL domain file defining actions and their effects.
    * *Example:* `plansys2_monitoring_simple_example/pddl/simple_example.pddl`
2. `plan.txt`: the plan file generated with `get plan` from `plansys2_terminal`.
    * *Example:* `plansys2_fake_sensing_utils/util/simple-domain/simple-domain-plan.txt`
3. `commands`: formatted as the commands files that can be found in the launch folders of the example packages (plansys2_terminal syntax)
    * *Example:* `plansys2_monitoring_simple_example/launch/commands`
4. `predicates.txt`: output predicate sequence file.
5. `functions.txt`: output function sequence file.

#### Example Output

**Predicate sequence (`predicates.txt`):**

```
car_at herbie wp0 false
```

**Function sequence (`functions.txt`):**

```
fuel_level herbie 0.33333333397725323
```

For an complete example output see folders:
- `plansys2_fake_sensing_utils/util/simple-domain`
- `plansys2_fake_sensing_utils/util/road-trip-domain`

---

The `FakeSensingSequences` class uses these files to **replay expected effects deterministically**, optionally with random failures.
