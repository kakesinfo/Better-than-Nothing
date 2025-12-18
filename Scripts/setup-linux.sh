#!/bin/bash

set -e

Update and upgrade the system
sudo apt update -y
sudo apt upgrade -y

sudo apt-get install -y \
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
    build-essential \
    cmake \
    ninja-build \
    python3 \
    clang \
    lld

sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt-get update

sudo apt-get install -y \
    vulkan-tools \
    libvulkan-dev \
    spirv-tools \
    libglfw3-dev \
    libglm-dev \
    doxygen \
    doxygen-gui \
    doxygen-doc

sudo apt-get install vulkan-validationlayers-dev -y || true
sudo apt-get install vulkan-utility-libraries-dev  -y || true

sudo apt-get install -y \
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
    libmpg123-dev

wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 18 all
rm llvm.sh

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100
sudo update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100

wget -q https://storage.googleapis.com/shaderc/artifacts/prod/graphics_shader_compiler/shaderc/linux/continuous_clang_release/419/20230623-090916/install.tgz
tar -xzf install.tgz
sudo cp ./install/bin/glslc /usr/local/bin/
rm -rf install.tgz ./install/

sudo apt autoremove -y
sudo apt-get clean

echo "export CC=/usr/bin/clang" >> ~/.bashrc
echo "export CXX=/usr/bin/clang++" >> ~/.bashrc