FROM osrf/ros:jazzy-desktop

ARG USERNAME=ubuntu
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN if ! id -u $USER_UID >/dev/null 2>&1; then \
    groupadd --gid $USER_GID $USERNAME && \
    useradd -s /bin/bash --uid $USER_UID --gid $USER_GID -m $USERNAME; \
  fi

RUN apt-get update && \
    apt-get install -y sudo && \
    echo "$USERNAME ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME && \
    chmod 0440 /etc/sudoers.d/$USERNAME

USER $USERNAME

RUN sudo apt-get update && sudo apt-get upgrade -y

RUN sudo apt-get install -y git python3-colcon-common-extensions

RUN rosdep update

RUN echo "source /opt/ros/${ROS_DISTRO}/setup.bash" >> ~/.bashrc

RUN mkdir -p ~/plansys2_ws/src

RUN git clone --branch effect-monitoring --single-branch \
    https://github.com/eugenio24/ros2_planning_system.git \
    ~/plansys2_ws/src/ros2_planning_system

RUN git clone \
    https://github.com/eugenio24/plansys2_monitoring_example.git \
    ~/plansys2_ws/src/plansys2_monitoring_example

RUN cd ~/plansys2_ws && \
    rosdep install --from-paths src --ignore-src -r -y

RUN /bin/bash -c "source /opt/ros/${ROS_DISTRO}/setup.bash && \
    cd ~/plansys2_ws && \
    colcon build --symlink-install"

RUN echo "source ~/plansys2_ws/install/setup.bash" >> ~/.bashrc

WORKDIR /home/${USERNAME}