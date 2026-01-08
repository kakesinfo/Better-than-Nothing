#!/bin/bash

set -e

apt-get update -y
apt-get upgrade -y

apt-get update && apt-get install -y \
    software-properties-common \
    autoconf \
    autogen \
    automake \
    wget \
    gpg \
    lsb-release \
    pkg-config \
    git \
    curl \
    ca-certificates \
    gnupg \
    lld \
    && rm -rf /var/lib/apt/lists/*

add-apt-repository ppa:ubuntu-toolchain-r/test -y

apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gcc-13 \
    g++-13 \
    clang \
    clang-format \
    clang-tidy \
    ninja-build \
    vulkan-tools \
    libvulkan-dev \
    spirv-tools \
    libglm-dev \
    python3 \
    libgtest-dev \
    doxygen \
    doxygen-gui \
    doxygen-doc \
    && rm -rf /var/lib/apt/lists/*

apt-get update && apt-get install vulkan-validationlayers-dev -y || true
apt-get update && apt-get install vulkan-utility-libraries-dev  -y || true

apt-get update && apt-get install -y \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libasound2-dev \
    libflac-dev \
    libogg-dev \
    libtool \
    libvorbis-dev \
    libopus-dev \
    libmp3lame-dev \
    libmpg123-dev \
    && rm -rf /var/lib/apt/lists/*

wget https://apt.llvm.org/llvm.sh \
    && chmod +x llvm.sh \
    && ./llvm.sh 18 all \
    && rm llvm.sh \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100
update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100

wget -q https://storage.googleapis.com/shaderc/artifacts/prod/graphics_shader_compiler/shaderc/linux/continuous_clang_release/419/20230623-090916/install.tgz
tar -xzf install.tgz
cp ./install/bin/glslc /usr/local/bin/
rm -rf install.tgz ./install/

apt-get autoremove -y
apt-get clean

echo "export CC=/usr/bin/clang" >> ~/.bashrc
echo "export CXX=/usr/bin/clang++" >> ~/.bashrc
