#!/bin/bash
set -e

apt-get update && apt-get install -y \
    software-properties-common \
    wget \
    gpg \
    lsb-release \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

add-apt-repository ppa:ubuntu-toolchain-r/test -y

apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    gcc-13 \
    g++-13 \
    git \
    pkg-config \
    libvulkan-dev \
    vulkan-tools \
    mesa-vulkan-drivers \
    glslang-tools \
    spirv-tools \
    libglm-dev \
    libgtest-dev \
    doxygen \
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
    && rm llvm.sh

if [ -f /usr/bin/gcc-13 ]; then
    update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
fi

if [ -f /usr/bin/clang-18 ]; then
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100
fi

apt-get autoremove -y && apt-get clean && rm -rf /var/lib/apt/lists/*

echo "export CC=/usr/bin/clang" >> ~/.bashrc
echo "export CXX=/usr/bin/clang++" >> ~/.bashrc
