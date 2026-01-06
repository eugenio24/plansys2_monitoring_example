# Copyright 2025 Intelligent Robotics Lab
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    # Get the launch directory
    example_dir = get_package_share_directory('plansys2_monitoring_numeric_example')
    namespace = LaunchConfiguration('namespace')

    declare_namespace_cmd = DeclareLaunchArgument(
        'namespace',
        default_value='',
        description='Namespace')

    plansys2_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            get_package_share_directory('plansys2_bringup'),
            'launch',
            'plansys2_bringup_launch_monolithic.py')),
        launch_arguments={
          'model_file': example_dir + '/pddl/road_trip_domain.pddl',
          'namespace': namespace,
          'action_bt_file': os.path.join(
            get_package_share_directory('plansys2_executor'),
            'behavior_trees',
            'plansys2_action_bt_monitored.xml')
          }.items())

    # Specify the actions
    move_cmd = Node(
        package='plansys2_monitoring_numeric_example',
        executable='move_action_node',
        name='move_action_node',
        namespace=namespace,
        output='screen',
        parameters=[])

    visit_cmd = Node(
        package='plansys2_monitoring_numeric_example',
        executable='visit_action_node',
        name='visit_action_node',
        namespace=namespace,
        output='screen',
        parameters=[])

    refuel_cmd = Node(
        package='plansys2_monitoring_numeric_example',
        executable='refuel_action_node',
        name='refuel_action_node',
        namespace=namespace,
        output='screen',
        parameters=[])

        
    # Create the launch description and populate
    ld = LaunchDescription()

    ld.add_action(declare_namespace_cmd)

    # Declare the launch options
    ld.add_action(plansys2_cmd)

    ld.add_action(move_cmd)
    ld.add_action(visit_cmd)
    ld.add_action(refuel_cmd)

    return ld
