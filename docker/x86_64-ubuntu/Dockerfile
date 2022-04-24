FROM ros:eloquent-ros-base-bionic
LABEL maintainer="qiao qiao.helloworld@gmail.com"

# setup keys
RUN apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys C1CF6E31E6BADE8868B172B4F42ED6FBAB17C654

# setup sources.list
RUN echo "deb http://packages.ros.org/ros/ubuntu bionic main" > /etc/apt/sources.list.d/ros1-latest.list

# install packages
ENV ROS1_DISTRO melodic
ENV ROS2_DISTRO eloquent
RUN sed -i 's#http://archive.ubuntu.com/#http://tw.archive.ubuntu.com/#' /etc/apt/sources.list \
  && apt-get update && apt-get upgrade -y && apt-get install -y --no-install-recommends \
  # common
  autoconf build-essential bzip2 clang clang-format cmake coreutils curl gdb git \
  google-mock googletest libboost-all-dev libeigen3-dev libgflags-dev libgoogle-glog-dev libgtest-dev \
  libsuitesparse-dev libtool lsb-release make minicom net-tools ninja-build \
  openssh-server pkg-config sudo valgrind vim wget zip zlib1g-dev \
  # development tools
  can-utils dbus-x11 debian-goodies gtk-recordmydesktop htop iputils-ping ipython ipython3 \
  jq keychain less meld moreutils nfs-common openbox pcl-tools psmisc python-pip python-pudb \
  python3-pip python3-pudb screen shellcheck silversearcher-ag tig tmux tree vim virtualenv \
  v4l-utils vlc x11-apps x11-xserver-utils xcompmgr xvfb zsh \
  # google cartographer dependencies
  clang g++ libcairo2-dev libcurl4-openssl-dev liblua5.2-dev python-sphinx \
  # education-common meta-packages
  apt-listchanges bash-completion bc bind9-host cfengine2 cifs-utils command-not-found \
  convmv cups cups-browsed debconf-utils debian-archive-keyring deborphan dhcping \
  dmidecode eject etherwake ethtool finger foomatic-db foomatic-db-engine fping hddtemp \
  hdparm hpijs-ppds hplip htop hwinfo iftop iotop iproute2 less libnss-myhostname \
  libpam-tmpdir libwww-perl lshw lsscsi man-db manpages mc memtest86+ mlocate mtools \
  mtr ncftp nictools-pci nmap nullidentd openbsd-inetd openssh-client pciutils printer-driver-hpijs \
  printer-driver-pnm2ppa procinfo psmisc python-gtk2 python-vte reportbug rsync \
  rsyslog screen strace sysfsutils tcpdump tcptraceroute traceroute unattended-upgrades \
  # ros packages
  ros-${ROS1_DISTRO}-desktop-full \
  # ros2 packages
  ros-${ROS2_DISTRO}-ros1-bridge \
  # delete temporary files to save space
  && apt-get autoclean \
  && apt-get autoremove \
  && rm -rf /var/lib/apt/lists/*

RUN pip3 install awscli pycodestyle yapf && \
  pip install scipy scikit-learn

# Install GTest/GMonk
RUN cd /usr/src/googletest && \
  mkdir build && \
  cd build && \
  cmake .. && \
  make install -j$(nproc --all)

# Install Ceres (pulled from googlecartographer)
ARG CERES_SOLVER_VERSION="1.14.0"
RUN git clone https://ceres-solver.googlesource.com/ceres-solver && \
  cd ceres-solver && \
  git checkout tags/${CERES_SOLVER_VERSION} && \
  mkdir build && \
  cd build && \
  cmake .. -G Ninja -DCXX11=ON && \
  ninja && \
  CTEST_OUTPUT_ON_FAILURE=1 ninja test && \
  ninja install && \
  cd ../.. && \
  rm -rf ceres-solver

# Install proto3 (pulled from googlecartographer)
ARG PROTOBUF_VERSION="v3.11.0"
RUN git clone https://github.com/google/protobuf.git && \
  cd protobuf && \
  git checkout tags/${PROTOBUF_VERSION} && \
  mkdir build && \
  cd build && \
  cmake -G Ninja \
  -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -Dprotobuf_BUILD_TESTS=OFF \
  ../cmake && \
  ninja && \
  ninja install && \
  cd ../.. && \
  rm -rf protobuf

# GDB
RUN echo "kernel.yama.ptrace_scope = 0" > /etc/sysctl.d/10-ptrace.conf

# OpenSSH
RUN mkdir /var/run/sshd && \
  sed -ri 's/UsePAM yes/#UsePAM yes/g' /etc/ssh/sshd_config && \
  echo X11UseLocalhost no >> /etc/ssh/sshd_config

WORKDIR /root
