FROM arm32v7/ubuntu:20.04

LABEL maintainer="qiao qiao.helloworld@gmail.com"

ENV LANG=en_US.UTF-8 \ 
    LANGUAGE=en_US:en \
    LC_ALL=en_US.UTF-8 \
    CMAKE_VERSION_MAJOR_MINOR=3.22 \
    CMAKE_VERSION_FULL=3.22.2 \
    PYENV_ROOT=/opt/pyenv \
    PYTHON_VERSION=3.7.5 \
    PATH=/opt/pyenv/shims:${PATH} \
    DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y sudo openssl apt-utils locales \ 
    && rm -rf /var/lib/apt/lists/*

RUN dpkg-reconfigure locales --frontend noninteractive \ 
    && locale-gen "en_US.UTF-8" \ 
    && update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8

RUN apt-get update && apt-get -qq install -y --no-install-recommends --no-install-suggests \
    autoconf-archive \
    binutils \
    ca-certificates \
    curl \
    dh-autoreconf \
    g++-multilib \
    git \
    libbz2-dev \
    libc6-dev \
    libffi-dev \
    libgmp-dev \
    libgtk2.0-dev \
    liblzma-dev \
    libmpc-dev \
    libmpfr-dev \
    libncurses5-dev \
    libncursesw5-dev \
    libreadline-dev \
    libsqlite3-dev \
    libssl-dev \
    nasm \
    ninja-build \
    pkg-config \
    subversion \
    sudo \  
    wget \
    xz-utils \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

RUN wget \
    https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION_FULL}/cmake-${CMAKE_VERSION_FULL}.tar.gz \
    && tar -xzf cmake-${CMAKE_VERSION_FULL}.tar.gz \
    && rm -rf cmake-${CMAKE_VERSION_FULL}.tar.gz \
    && cd cmake-${CMAKE_VERSION_FULL} \
    && ./bootstrap \
    && make -j$(nproc) \
    && make install \
    && cd .. \
    && rm -rf cmake-${CMAKE_VERSION_FULL}

RUN wget --no-check-certificate --quiet -O /tmp/pyenv-installer https://github.com/pyenv/pyenv-installer/raw/master/bin/pyenv-installer \
    && chmod +x /tmp/pyenv-installer \
    && /tmp/pyenv-installer \
    && rm /tmp/pyenv-installer \
    && update-alternatives --install /usr/bin/pyenv pyenv /opt/pyenv/bin/pyenv 100 \
    && PYTHON_CONFIGURE_OPTS="--enable-shared" pyenv install ${PYTHON_VERSION} \
    && pyenv global ${PYTHON_VERSION} \
    && ln -s /usr/lib/python3/dist-packages/lsb_release.py /opt/pyenv/versions/3.7.5/lib/python3.7/lsb_release.py \
    && pip install -q --upgrade --no-cache-dir pip \
    && pip install -q --no-cache-dir conan conan-package-tools cmake==${CMAKE_VERSION_FULL} \
    && find /opt/pyenv -iname __pycache__ -print0 | xargs -0 rm -rf \
    && update-alternatives --install /usr/bin/python python /opt/pyenv/shims/python 100 \
    && update-alternatives --install /usr/bin/python3 python3 /opt/pyenv/shims/python3 100 \
    && update-alternatives --install /usr/bin/pip pip /opt/pyenv/shims/pip 100 \
    && update-alternatives --install /usr/bin/pip3 pip3 /opt/pyenv/shims/pip3 100

# guard avoid running build script when conanfile remains the same
ARG CONANFILE_CRC32

COPY  conanfile.txt /tmp/conanfile.txt

# RUN conan install /tmp/conanfile.txt --build=missing \
#     && rm -rf /tmp/conanfile.txt

