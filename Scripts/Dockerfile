FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
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
    && add-apt-repository ppa:ubuntu-toolchain-r/test -y \
    && apt-get update \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-13 \
    g++-13 \
    cmake \
    ninja-build \
    python3 \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
    vulkan-tools \
    libvulkan-dev \
    vulkan-validationlayers-dev \
    spirv-tools \
    libglfw3-dev \
    libglm-dev \
    doxygen \
    doxygen-gui \
    doxygen-doc \
    && rm -rf /var/lib/apt/lists/*

RUN apt-get update && apt-get install -y \
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

RUN wget https://apt.llvm.org/llvm.sh \
    && chmod +x llvm.sh \
    && ./llvm.sh 18 all \
    && rm llvm.sh \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100 \
    && update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100 \
    && update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100

RUN wget -q https://storage.googleapis.com/shaderc/artifacts/prod/graphics_shader_compiler/shaderc/linux/continuous_clang_release/419/20230623-090916/install.tgz \
    && tar -xzf install.tgz \
    && cp ./install/bin/glslc /usr/local/bin/ \
    && rm -rf install.tgz ./install/

ENV CC=/usr/bin/clang
ENV CXX=/usr/bin/clang++

WORKDIR /root

CMD ["/bin/bash"]