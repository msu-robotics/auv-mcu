# Настрока среды разработки

Для работы с microros в platformio необходимо настроить среду разработки согластно инструкции https://github.com/micro-ROS/micro_ros_platformio


# Архитектура системы

Основная платформа - ESP32, так как этот микроконтроллер оснащен двухядерным процессором, то
задачи работы ROS нод будут обрабатыватся одним ядром, а задачи опроса переферии будут работать на другом ядре


## Ros2 Docker

Для работы с ROS2 будет использоваться docker, это позволит удобно осуществлять деплой на
целевую систему, осуществлять быструю настройку среды разработки и ее идентичность целеой системе

Для этого будет необходимо проделать следующее:

- Создать в корне прокта папки `ros2_ws` и `microros_ws`
- Подготовить  локальный образ для разработки на основе `osrf/ros:humble-desktop`
- Установить дополнительные зависимости в образ
- Собрать microros и сделать checkpoint

Запусти локальный образ:

```shell
docker run -it \
  --name ros2-humble-dev-container \
  --privileged \
  -v ros2_ws:/root/ros2_ws \
  -v microros_ws:/root/microros_ws \
  -v /dev:/dev \
  osrf/ros:humble-desktop
```

Установи и скачай зависимости:

```shell
cd root/microros_ws
git clone -b $ROS_DISTRO https://github.com/micro-ROS/micro_ros_setup.git src/micro_ros_setup
sudo apt update && sudo apt install python3-rosdep2 python3-pip
```


Собери microros:
```shell
sudo rosdep update
rosdep install --from-paths src --ignore-src -y
colcon build
source install/local_setup.bash
ros2 run micro_ros_setup create_agent_ws.sh
ros2 run micro_ros_setup build_agent.sh
source ~/microros_ws/install/local_setup.bash
```

Сделай checkpoint:
```shell
docker checkpoint create ros2-humble-dev-container installed-microros
```

```shell
ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0
```


<details>
  <summary>Пример вывода</summary>

```shell
➜  auv-mcu git:(main) ✗ docker run --rm -it \
  --name ros2-humble-dev-container \
  --env="DISPLAY=host.docker.internal:0" \
  --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
  -v ros2_ws:/root/ros2_ws \
  -v microros_ws:/root/microros_ws \
  -v /dev/tty.usbserial-0001:/dev/tty.usbserial-0001 \
  osrf/ros:humble-desktop
WARNING: The requested image's platform (linux/amd64) does not match the detected host platform (linux/arm64/v8) and no specific platform was requested
root@e23720deeb17:/# ls
bin  boot  dev  etc  home  lib  lib32  lib64  libx32  log  media  mnt  opt  proc  root  ros_entrypoint.sh  run  sbin  srv  sys  tmp  usr  var
root@e23720deeb17:/# cd root/
root@e23720deeb17:~# ls
microros_ws  ros2_ws
root@e23720deeb17:~# cd microros_ws/
root@e23720deeb17:~/microros_ws# ls
root@e23720deeb17:~/microros_ws# git clone -b $ROS_DISTRO https://github.com/micro-ROS/micro_ros_setup.git src/micro_ros_setup
Cloning into 'src/micro_ros_setup'...
remote: Enumerating objects: 4093, done.
remote: Counting objects: 100% (353/353), done.
remote: Compressing objects: 100% (229/229), done.
remote: Total 4093 (delta 267), reused 143 (delta 123), pack-reused 3740 (from 4)
Receiving objects: 100% (4093/4093), 962.89 KiB | 1.04 MiB/s, done.
Resolving deltas: 100% (2806/2806), done.
root@e23720deeb17:~/microros_ws# sudo apt update
Get:1 http://security.ubuntu.com/ubuntu jammy-security InRelease [129 kB]
Get:2 http://archive.ubuntu.com/ubuntu jammy InRelease [270 kB]
Get:3 http://packages.ros.org/ros2/ubuntu jammy InRelease [4682 B]
Get:4 http://archive.ubuntu.com/ubuntu jammy-updates InRelease [128 kB]
Get:5 http://archive.ubuntu.com/ubuntu jammy-backports InRelease [127 kB]
Get:6 http://security.ubuntu.com/ubuntu jammy-security/universe amd64 Packages [1253 kB]
Get:7 http://packages.ros.org/ros2/ubuntu jammy/main Sources [1700 kB]
Get:8 http://archive.ubuntu.com/ubuntu jammy/universe amd64 Packages [17.5 MB]
Get:9 http://security.ubuntu.com/ubuntu jammy-security/main amd64 Packages [3024 kB]
Get:10 http://security.ubuntu.com/ubuntu jammy-security/restricted amd64 Packages [4532 kB]
Get:11 http://security.ubuntu.com/ubuntu jammy-security/multiverse amd64 Packages [47.7 kB]
Get:12 http://packages.ros.org/ros2/ubuntu jammy/main amd64 Packages [1674 kB]
Get:13 http://archive.ubuntu.com/ubuntu jammy/restricted amd64 Packages [164 kB]
Get:14 http://archive.ubuntu.com/ubuntu jammy/multiverse amd64 Packages [266 kB]
Get:15 http://archive.ubuntu.com/ubuntu jammy/main amd64 Packages [1792 kB]
Get:16 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 Packages [1561 kB]
Get:17 http://archive.ubuntu.com/ubuntu jammy-updates/restricted amd64 Packages [4703 kB]
Get:18 http://archive.ubuntu.com/ubuntu jammy-updates/main amd64 Packages [3340 kB]
Get:19 http://archive.ubuntu.com/ubuntu jammy-updates/multiverse amd64 Packages [55.7 kB]
Get:20 http://archive.ubuntu.com/ubuntu jammy-backports/universe amd64 Packages [35.2 kB]
Get:21 http://archive.ubuntu.com/ubuntu jammy-backports/main amd64 Packages [83.2 kB]
Fetched 42.4 MB in 44s (966 kB/s)
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
298 packages can be upgraded. Run 'apt list --upgradable' to see them.
root@e23720deeb17:~/microros_ws# sudo apt install python3-rosdep2
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following package was automatically installed and is no longer required:
  sudo
Use 'sudo apt autoremove' to remove it.
The following additional packages will be installed:
  python3-catkin-pkg python3-rosdistro python3-rospkg
The following packages will be REMOVED:
  python3-rosdep python3-rosdep-modules
The following NEW packages will be installed:
  python3-catkin-pkg python3-rosdep2 python3-rosdistro python3-rospkg
0 upgraded, 4 newly installed, 2 to remove and 298 not upgraded.
Need to get 68.9 kB of archives.
After this operation, 77.8 kB of additional disk space will be used.
Do you want to continue? [Y/n] y
Get:1 http://archive.ubuntu.com/ubuntu jammy/universe amd64 python3-rosdep2 all 0.21.0-2 [58.7 kB]
Get:2 http://packages.ros.org/ros2/ubuntu jammy/main amd64 python3-catkin-pkg all 1.0.0-100 [3920 B]
Get:3 http://packages.ros.org/ros2/ubuntu jammy/main amd64 python3-rosdistro all 1.0.1-100 [3724 B]
Get:4 http://packages.ros.org/ros2/ubuntu jammy/main amd64 python3-rospkg all 1.6.0-100 [2528 B]
Fetched 68.9 kB in 1s (77.1 kB/s)
debconf: delaying package configuration, since apt-utils is not installed
(Reading database ... 114747 files and directories currently installed.)
Removing python3-rosdep (0.25.1-1) ...
Removing python3-rosdep-modules (0.25.1-1) ...
Selecting previously unselected package python3-catkin-pkg.
(Reading database ... 114680 files and directories currently installed.)
Preparing to unpack .../python3-catkin-pkg_1.0.0-100_all.deb ...
Unpacking python3-catkin-pkg (1.0.0-100) ...
Selecting previously unselected package python3-rosdistro.
Preparing to unpack .../python3-rosdistro_1.0.1-100_all.deb ...
Unpacking python3-rosdistro (1.0.1-100) ...
Selecting previously unselected package python3-rospkg.
Preparing to unpack .../python3-rospkg_1.6.0-100_all.deb ...
Unpacking python3-rospkg (1.6.0-100) ...
Selecting previously unselected package python3-rosdep2.
Preparing to unpack .../python3-rosdep2_0.21.0-2_all.deb ...
Unpacking python3-rosdep2 (0.21.0-2) ...
Setting up python3-rospkg (1.6.0-100) ...
Setting up python3-catkin-pkg (1.0.0-100) ...
Setting up python3-rosdistro (1.0.1-100) ...
Setting up python3-rosdep2 (0.21.0-2) ...
root@e23720deeb17:~/microros_ws# sudo apt update && rosdep update
Hit:1 http://archive.ubuntu.com/ubuntu jammy InRelease
Hit:2 http://packages.ros.org/ros2/ubuntu jammy InRelease
Hit:3 http://security.ubuntu.com/ubuntu jammy-security InRelease
Hit:4 http://archive.ubuntu.com/ubuntu jammy-updates InRelease
Hit:5 http://archive.ubuntu.com/ubuntu jammy-backports InRelease
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
298 packages can be upgraded. Run 'apt list --upgradable' to see them.
reading in sources list data from /etc/ros/rosdep/sources.list.d
Warning: running 'rosdep update' as root is not recommended.
  You should run 'sudo rosdep fix-permissions' and invoke 'rosdep update' again without sudo.
Hit file:///usr/share/python3-rosdep2/debian.yaml
Hit https://raw.githubusercontent.com/ros/rosdistro/master/rosdep/osx-homebrew.yaml
Hit https://raw.githubusercontent.com/ros/rosdistro/master/rosdep/base.yaml
Hit https://raw.githubusercontent.com/ros/rosdistro/master/rosdep/python.yaml
Hit https://raw.githubusercontent.com/ros/rosdistro/master/rosdep/ruby.yaml
Hit https://raw.githubusercontent.com/ros/rosdistro/master/releases/fuerte.yaml
Query rosdistro index https://raw.githubusercontent.com/ros/rosdistro/master/index-v4.yaml
Skip end-of-life distro "ardent"
Skip end-of-life distro "bouncy"
Skip end-of-life distro "crystal"
Skip end-of-life distro "dashing"
Skip end-of-life distro "eloquent"
Skip end-of-life distro "foxy"
Skip end-of-life distro "galactic"
Skip end-of-life distro "groovy"
Add distro "humble"
Skip end-of-life distro "hydro"
Skip end-of-life distro "indigo"
Skip end-of-life distro "iron"
Skip end-of-life distro "jade"
Add distro "jazzy"
Add distro "kilted"
Skip end-of-life distro "kinetic"
Skip end-of-life distro "lunar"
Skip end-of-life distro "melodic"
Skip end-of-life distro "noetic"
Add distro "rolling"
updated cache in /root/.ros/rosdep/sources.cache
root@e23720deeb17:~/microros_ws# rosdep install --from-paths src --ignore-src -y
executing command [apt-get install -y libasio-dev]
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following package was automatically installed and is no longer required:
  sudo
Use 'apt autoremove' to remove it.
The following NEW packages will be installed:
  libasio-dev
0 upgraded, 1 newly installed, 0 to remove and 298 not upgraded.
Need to get 352 kB of archives.
After this operation, 4618 kB of additional disk space will be used.
Get:1 http://archive.ubuntu.com/ubuntu jammy/universe amd64 libasio-dev all 1:1.18.1-1 [352 kB]
Fetched 352 kB in 1s (247 kB/s)
debconf: delaying package configuration, since apt-utils is not installed
Selecting previously unselected package libasio-dev.
(Reading database ... 114785 files and directories currently installed.)
Preparing to unpack .../libasio-dev_1%3a1.18.1-1_all.deb ...
Unpacking libasio-dev (1:1.18.1-1) ...
Setting up libasio-dev (1:1.18.1-1) ...
executing command [apt-get install -y flex]
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following package was automatically installed and is no longer required:
  sudo
Use 'apt autoremove' to remove it.
The following additional packages will be installed:
  libfl-dev libfl2
Suggested packages:
  bison flex-doc
The following NEW packages will be installed:
  flex libfl-dev libfl2
0 upgraded, 3 newly installed, 0 to remove and 298 not upgraded.
Need to get 324 kB of archives.
After this operation, 1148 kB of additional disk space will be used.
Get:1 http://archive.ubuntu.com/ubuntu jammy/main amd64 flex amd64 2.6.4-8build2 [307 kB]
Get:2 http://archive.ubuntu.com/ubuntu jammy/main amd64 libfl2 amd64 2.6.4-8build2 [10.7 kB]
Get:3 http://archive.ubuntu.com/ubuntu jammy/main amd64 libfl-dev amd64 2.6.4-8build2 [6236 B]
Fetched 324 kB in 1s (223 kB/s)
debconf: delaying package configuration, since apt-utils is not installed
Selecting previously unselected package flex.
(Reading database ... 115376 files and directories currently installed.)
Preparing to unpack .../flex_2.6.4-8build2_amd64.deb ...
Unpacking flex (2.6.4-8build2) ...
Selecting previously unselected package libfl2:amd64.
Preparing to unpack .../libfl2_2.6.4-8build2_amd64.deb ...
Unpacking libfl2:amd64 (2.6.4-8build2) ...
Selecting previously unselected package libfl-dev:amd64.
Preparing to unpack .../libfl-dev_2.6.4-8build2_amd64.deb ...
Unpacking libfl-dev:amd64 (2.6.4-8build2) ...
Setting up flex (2.6.4-8build2) ...
Setting up libfl2:amd64 (2.6.4-8build2) ...
Setting up libfl-dev:amd64 (2.6.4-8build2) ...
Processing triggers for libc-bin (2.35-0ubuntu3.10) ...
executing command [apt-get install -y bison]
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following package was automatically installed and is no longer required:
  sudo
Use 'apt autoremove' to remove it.
Suggested packages:
  bison-doc
The following NEW packages will be installed:
  bison
0 upgraded, 1 newly installed, 0 to remove and 298 not upgraded.
Need to get 748 kB of archives.
After this operation, 2519 kB of additional disk space will be used.
Get:1 http://archive.ubuntu.com/ubuntu jammy/main amd64 bison amd64 2:3.8.2+dfsg-1build1 [748 kB]
Fetched 748 kB in 2s (483 kB/s)
debconf: delaying package configuration, since apt-utils is not installed
Selecting previously unselected package bison.
(Reading database ... 115466 files and directories currently installed.)
Preparing to unpack .../bison_2%3a3.8.2+dfsg-1build1_amd64.deb ...
Unpacking bison (2:3.8.2+dfsg-1build1) ...
Setting up bison (2:3.8.2+dfsg-1build1) ...
update-alternatives: using /usr/bin/bison.yacc to provide /usr/bin/yacc (yacc) in auto mode
update-alternatives: warning: skip creation of /usr/share/man/man1/yacc.1.gz because associated file /usr/share/man/man1/bison.yacc.1.gz (of link group yacc) doesn't exist
executing command [apt-get install -y libncurses-dev]
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following package was automatically installed and is no longer required:
  sudo
Use 'apt autoremove' to remove it.
Suggested packages:
  ncurses-doc
The following NEW packages will be installed:
  libncurses-dev
0 upgraded, 1 newly installed, 0 to remove and 298 not upgraded.
Need to get 381 kB of archives.
After this operation, 2407 kB of additional disk space will be used.
Get:1 http://archive.ubuntu.com/ubuntu jammy-updates/main amd64 libncurses-dev amd64 6.3-2ubuntu0.1 [381 kB]
Fetched 381 kB in 1s (306 kB/s)
debconf: delaying package configuration, since apt-utils is not installed
Selecting previously unselected package libncurses-dev:amd64.
(Reading database ... 115582 files and directories currently installed.)
Preparing to unpack .../libncurses-dev_6.3-2ubuntu0.1_amd64.deb ...
Unpacking libncurses-dev:amd64 (6.3-2ubuntu0.1) ...
Setting up libncurses-dev:amd64 (6.3-2ubuntu0.1) ...
executing command [apt-get install -y usbutils]
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following package was automatically installed and is no longer required:
  sudo
Use 'apt autoremove' to remove it.
The following NEW packages will be installed:
  usbutils
0 upgraded, 1 newly installed, 0 to remove and 298 not upgraded.
Need to get 85.6 kB of archives.
After this operation, 333 kB of additional disk space will be used.
Get:1 http://archive.ubuntu.com/ubuntu jammy/main amd64 usbutils amd64 1:014-1build1 [85.6 kB]
Fetched 85.6 kB in 1s (117 kB/s)
debconf: delaying package configuration, since apt-utils is not installed
Selecting previously unselected package usbutils.
(Reading database ... 115671 files and directories currently installed.)
Preparing to unpack .../usbutils_1%3a014-1build1_amd64.deb ...
Unpacking usbutils (1:014-1build1) ...
Setting up usbutils (1:014-1build1) ...
executing command [apt-get install -y clang-tidy]
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following package was automatically installed and is no longer required:
  sudo
Use 'apt autoremove' to remove it.
The following additional packages will be installed:
  binfmt-support clang-14 clang-tidy-14 clang-tools-14 lib32gcc-s1 lib32stdc++6 libc6-i386 libclang-common-14-dev libclang-cpp14 libgc1 libobjc-11-dev libobjc4 libpfm4 libpipeline1 libz3-dev llvm-14
  llvm-14-dev llvm-14-linker-tools llvm-14-runtime llvm-14-tools
Suggested packages:
  clang-14-doc llvm-14-doc
The following NEW packages will be installed:
  binfmt-support clang-14 clang-tidy clang-tidy-14 clang-tools-14 lib32gcc-s1 lib32stdc++6 libc6-i386 libclang-common-14-dev libclang-cpp14 libgc1 libobjc-11-dev libobjc4 libpfm4 libpipeline1 libz3-dev
  llvm-14 llvm-14-dev llvm-14-linker-tools llvm-14-runtime llvm-14-tools
0 upgraded, 21 newly installed, 0 to remove and 298 not upgraded.
Need to get 83.9 MB of archives.
After this operation, 530 MB of additional disk space will be used.
Get:1 http://archive.ubuntu.com/ubuntu jammy/main amd64 libpipeline1 amd64 1.5.5-1 [23.5 kB]
Get:2 http://archive.ubuntu.com/ubuntu jammy/main amd64 binfmt-support amd64 2.2.1-2 [55.8 kB]
Get:3 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 libclang-cpp14 amd64 1:14.0.0-1ubuntu1.1 [12.1 MB]
Get:4 http://archive.ubuntu.com/ubuntu jammy/main amd64 libgc1 amd64 1:8.0.6-1.1build1 [96.8 kB]
Get:5 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 libobjc4 amd64 12.3.0-1ubuntu1~22.04 [48.6 kB]
Get:6 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 libobjc-11-dev amd64 11.4.0-1ubuntu1~22.04 [196 kB]
Get:7 http://archive.ubuntu.com/ubuntu jammy-updates/main amd64 libc6-i386 amd64 2.35-0ubuntu3.10 [2837 kB]
Get:8 http://archive.ubuntu.com/ubuntu jammy-updates/main amd64 lib32gcc-s1 amd64 12.3.0-1ubuntu1~22.04 [63.9 kB]
Get:9 http://archive.ubuntu.com/ubuntu jammy-updates/main amd64 lib32stdc++6 amd64 12.3.0-1ubuntu1~22.04 [740 kB]
Get:10 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 libclang-common-14-dev amd64 1:14.0.0-1ubuntu1.1 [5975 kB]
Get:11 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 llvm-14-linker-tools amd64 1:14.0.0-1ubuntu1.1 [1355 kB]
Get:12 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 clang-14 amd64 1:14.0.0-1ubuntu1.1 [81.2 kB]
Get:13 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 clang-tools-14 amd64 1:14.0.0-1ubuntu1.1 [6962 kB]
Get:14 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 clang-tidy-14 amd64 1:14.0.0-1ubuntu1.1 [1626 kB]
Get:15 http://archive.ubuntu.com/ubuntu jammy/universe amd64 clang-tidy amd64 1:14.0-55~exp2 [3456 B]
Get:16 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 llvm-14-runtime amd64 1:14.0.0-1ubuntu1.1 [484 kB]
Get:17 http://archive.ubuntu.com/ubuntu jammy-updates/main amd64 libpfm4 amd64 4.11.1+git32-gd0b85fb-1ubuntu0.1 [345 kB]
Get:18 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 llvm-14 amd64 1:14.0.0-1ubuntu1.1 [12.7 MB]
Get:19 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 llvm-14-tools amd64 1:14.0.0-1ubuntu1.1 [404 kB]
Get:20 http://archive.ubuntu.com/ubuntu jammy/universe amd64 libz3-dev amd64 4.8.12-1 [72.2 kB]
Get:21 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 llvm-14-dev amd64 1:14.0.0-1ubuntu1.1 [37.8 MB]
Fetched 83.9 MB in 32s (2617 kB/s)
debconf: delaying package configuration, since apt-utils is not installed
Selecting previously unselected package libpipeline1:amd64.
(Reading database ... 115683 files and directories currently installed.)
Preparing to unpack .../00-libpipeline1_1.5.5-1_amd64.deb ...
Unpacking libpipeline1:amd64 (1.5.5-1) ...
Selecting previously unselected package binfmt-support.
Preparing to unpack .../01-binfmt-support_2.2.1-2_amd64.deb ...
Unpacking binfmt-support (2.2.1-2) ...
Selecting previously unselected package libclang-cpp14.
Preparing to unpack .../02-libclang-cpp14_1%3a14.0.0-1ubuntu1.1_amd64.deb ...
Unpacking libclang-cpp14 (1:14.0.0-1ubuntu1.1) ...
Selecting previously unselected package libgc1:amd64.
Preparing to unpack .../03-libgc1_1%3a8.0.6-1.1build1_amd64.deb ...
Unpacking libgc1:amd64 (1:8.0.6-1.1build1) ...
Selecting previously unselected package libobjc4:amd64.
Preparing to unpack .../04-libobjc4_12.3.0-1ubuntu1~22.04_amd64.deb ...
Unpacking libobjc4:amd64 (12.3.0-1ubuntu1~22.04) ...
Selecting previously unselected package libobjc-11-dev:amd64.
Preparing to unpack .../05-libobjc-11-dev_11.4.0-1ubuntu1~22.04_amd64.deb ...
Unpacking libobjc-11-dev:amd64 (11.4.0-1ubuntu1~22.04) ...
Selecting previously unselected package libc6-i386.
Preparing to unpack .../06-libc6-i386_2.35-0ubuntu3.10_amd64.deb ...
Unpacking libc6-i386 (2.35-0ubuntu3.10) ...
Selecting previously unselected package lib32gcc-s1.
Preparing to unpack .../07-lib32gcc-s1_12.3.0-1ubuntu1~22.04_amd64.deb ...
Unpacking lib32gcc-s1 (12.3.0-1ubuntu1~22.04) ...
Selecting previously unselected package lib32stdc++6.
Preparing to unpack .../08-lib32stdc++6_12.3.0-1ubuntu1~22.04_amd64.deb ...
Unpacking lib32stdc++6 (12.3.0-1ubuntu1~22.04) ...
Selecting previously unselected package libclang-common-14-dev.
Preparing to unpack .../09-libclang-common-14-dev_1%3a14.0.0-1ubuntu1.1_amd64.deb ...
Unpacking libclang-common-14-dev (1:14.0.0-1ubuntu1.1) ...
Selecting previously unselected package llvm-14-linker-tools.
Preparing to unpack .../10-llvm-14-linker-tools_1%3a14.0.0-1ubuntu1.1_amd64.deb ...
Unpacking llvm-14-linker-tools (1:14.0.0-1ubuntu1.1) ...
Selecting previously unselected package clang-14.
Preparing to unpack .../11-clang-14_1%3a14.0.0-1ubuntu1.1_amd64.deb ...
Unpacking clang-14 (1:14.0.0-1ubuntu1.1) ...
Selecting previously unselected package clang-tools-14.
Preparing to unpack .../12-clang-tools-14_1%3a14.0.0-1ubuntu1.1_amd64.deb ...
Unpacking clang-tools-14 (1:14.0.0-1ubuntu1.1) ...
Selecting previously unselected package clang-tidy-14.
Preparing to unpack .../13-clang-tidy-14_1%3a14.0.0-1ubuntu1.1_amd64.deb ...
Unpacking clang-tidy-14 (1:14.0.0-1ubuntu1.1) ...
Selecting previously unselected package clang-tidy.
Preparing to unpack .../14-clang-tidy_1%3a14.0-55~exp2_amd64.deb ...
Unpacking clang-tidy (1:14.0-55~exp2) ...
Selecting previously unselected package llvm-14-runtime.
Preparing to unpack .../15-llvm-14-runtime_1%3a14.0.0-1ubuntu1.1_amd64.deb ...
Unpacking llvm-14-runtime (1:14.0.0-1ubuntu1.1) ...
Selecting previously unselected package libpfm4:amd64.
Preparing to unpack .../16-libpfm4_4.11.1+git32-gd0b85fb-1ubuntu0.1_amd64.deb ...
Unpacking libpfm4:amd64 (4.11.1+git32-gd0b85fb-1ubuntu0.1) ...
Selecting previously unselected package llvm-14.
Preparing to unpack .../17-llvm-14_1%3a14.0.0-1ubuntu1.1_amd64.deb ...
Unpacking llvm-14 (1:14.0.0-1ubuntu1.1) ...
Selecting previously unselected package llvm-14-tools.
Preparing to unpack .../18-llvm-14-tools_1%3a14.0.0-1ubuntu1.1_amd64.deb ...
Unpacking llvm-14-tools (1:14.0.0-1ubuntu1.1) ...
Selecting previously unselected package libz3-dev:amd64.
Preparing to unpack .../19-libz3-dev_4.8.12-1_amd64.deb ...
Unpacking libz3-dev:amd64 (4.8.12-1) ...
Selecting previously unselected package llvm-14-dev.
Preparing to unpack .../20-llvm-14-dev_1%3a14.0.0-1ubuntu1.1_amd64.deb ...
Unpacking llvm-14-dev (1:14.0.0-1ubuntu1.1) ...
Setting up libpipeline1:amd64 (1.5.5-1) ...
Setting up libz3-dev:amd64 (4.8.12-1) ...
Setting up libpfm4:amd64 (4.11.1+git32-gd0b85fb-1ubuntu0.1) ...
Setting up llvm-14-runtime (1:14.0.0-1ubuntu1.1) ...
Setting up binfmt-support (2.2.1-2) ...
invoke-rc.d: could not determine current runlevel
invoke-rc.d: policy-rc.d denied execution of restart.
Created symlink /etc/systemd/system/multi-user.target.wants/binfmt-support.service → /lib/systemd/system/binfmt-support.service.
Setting up libclang-cpp14 (1:14.0.0-1ubuntu1.1) ...
Setting up libgc1:amd64 (1:8.0.6-1.1build1) ...
Setting up libc6-i386 (2.35-0ubuntu3.10) ...
Setting up llvm-14-linker-tools (1:14.0.0-1ubuntu1.1) ...
Setting up llvm-14 (1:14.0.0-1ubuntu1.1) ...
Setting up llvm-14-tools (1:14.0.0-1ubuntu1.1) ...
Setting up libobjc4:amd64 (12.3.0-1ubuntu1~22.04) ...
Setting up lib32gcc-s1 (12.3.0-1ubuntu1~22.04) ...
Setting up lib32stdc++6 (12.3.0-1ubuntu1~22.04) ...
Setting up libclang-common-14-dev (1:14.0.0-1ubuntu1.1) ...
Setting up llvm-14-dev (1:14.0.0-1ubuntu1.1) ...
Setting up libobjc-11-dev:amd64 (11.4.0-1ubuntu1~22.04) ...
Setting up clang-14 (1:14.0.0-1ubuntu1.1) ...
Setting up clang-tools-14 (1:14.0.0-1ubuntu1.1) ...
Setting up clang-tidy-14 (1:14.0.0-1ubuntu1.1) ...
Setting up clang-tidy (1:14.0-55~exp2) ...
Processing triggers for libc-bin (2.35-0ubuntu3.10) ...
#All required rosdeps installed successfully
root@e23720deeb17:~/microros_ws# sudo apt-get install python3-pip
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
The following package was automatically installed and is no longer required:
  sudo
Use 'sudo apt autoremove' to remove it.
The following additional packages will be installed:
  python3-wheel
The following NEW packages will be installed:
  python3-pip python3-wheel
0 upgraded, 2 newly installed, 0 to remove and 298 not upgraded.
Need to get 1338 kB of archives.
After this operation, 7178 kB of additional disk space will be used.
Do you want to continue? [Y/n] y
Get:1 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 python3-wheel all 0.37.1-2ubuntu0.22.04.1 [32.0 kB]
Get:2 http://archive.ubuntu.com/ubuntu jammy-updates/universe amd64 python3-pip all 22.0.2+dfsg-1ubuntu0.5 [1306 kB]
Fetched 1338 kB in 2s (621 kB/s)
debconf: delaying package configuration, since apt-utils is not installed
Selecting previously unselected package python3-wheel.
(Reading database ... 119522 files and directories currently installed.)
Preparing to unpack .../python3-wheel_0.37.1-2ubuntu0.22.04.1_all.deb ...
Unpacking python3-wheel (0.37.1-2ubuntu0.22.04.1) ...
Selecting previously unselected package python3-pip.
Preparing to unpack .../python3-pip_22.0.2+dfsg-1ubuntu0.5_all.deb ...
Unpacking python3-pip (22.0.2+dfsg-1ubuntu0.5) ...
Setting up python3-wheel (0.37.1-2ubuntu0.22.04.1) ...
Setting up python3-pip (22.0.2+dfsg-1ubuntu0.5) ...
root@e23720deeb17:~/microros_ws# colcon build
Starting >>> micro_ros_setup
Finished <<< micro_ros_setup [2.38s]

Summary: 1 package finished [2.52s]
root@e23720deeb17:~/microros_ws# source install/local_setup.bash
root@e23720deeb17:~/microros_ws# ros2 run micro_ros_setup create_agent_ws.sh
..
=== ./uros/micro-ROS-Agent (git) ===
Cloning into '.'...
=== ./uros/micro_ros_msgs (git) ===
Cloning into '.'...
#All required rosdeps installed successfully
root@e23720deeb17:~/microros_ws# ros2 run micro_ros_setup build_agent.sh
Building micro-ROS Agent
Starting >>> micro_ros_msgs
Finished <<< micro_ros_msgs [8.61s]
Starting >>> micro_ros_agent
[Processing: micro_ros_agent]
--- stderr: micro_ros_agent
Cloning into 'xrceagent'...
HEAD is now at 57d0862 Release v2.4.2
CMake Warning (dev) at /usr/share/cmake-3.22/Modules/FindPackageHandleStandardArgs.cmake:438 (message):
  The package name passed to `find_package_handle_standard_args` (tinyxml2)
  does not match the name of the calling package (TinyXML2).  This can lead
  to problems in calling code that expects `find_package` result variables
  (e.g., `_FOUND`) to follow a certain pattern.
Call Stack (most recent call first):
  cmake/modules/FindTinyXML2.cmake:40 (find_package_handle_standard_args)
  /opt/ros/humble/share/fastrtps/cmake/fastrtps-config.cmake:51 (find_package)
  CMakeLists.txt:153 (find_package)
This warning is for project developers.  Use -Wno-dev to suppress it.

---
Finished <<< micro_ros_agent [34.6s]

Summary: 2 packages finished [43.4s]
  1 package had stderr output: micro_ros_agent
root@e23720deeb17:~/microros_ws# source ~/microros_ws/install/local_setup.bash
root@e23720deeb17:~/microros_ws#
```

</details>