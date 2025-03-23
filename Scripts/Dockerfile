FROM ubuntu:22.04

RUN apt update -y
RUN apt upgrade -y

# Install basic dependencies
RUN apt install -y  autoconf autogen automake build-essential
RUN apt install -y  cmake wget pkg-config

# Install vulkan dependencies
RUN apt install -y  vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools

# Install glfw dependencies
RUN apt install -y  libglfw3-dev libglm-dev

# Install python3 dependencies
RUN apt install -y  python3

# Install doxygen dependencies
RUN apt install -y  doxygen doxygen-gui doxygen-doc

# Install audio dependencies
RUN apt install -y  libxinerama-dev \
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

# Install llvm/clang dependencies
RUN apt install -y  lsb-release software-properties-common gnupg

# Install llvm/clang 18
RUN wget https://apt.llvm.org/llvm.sh \
    && chmod u+x llvm.sh \
    && ./llvm.sh 18 \
    && rm llvm.sh

RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-18 100
RUN update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-18 100

# Install glslc
RUN wget https://storage.googleapis.com/shaderc/artifacts/prod/graphics_shader_compiler/shaderc/linux/continuous_clang_release/419/20230623-090916/install.tgz \
    && tar -xvzf install.tgz \
    && cp ./install/bin/glslc /usr/local/bin/ \
    && rm -rf install.tgz ./install/

RUN rm -rf /var/lib/apt/lists/*

WORKDIR /root

CMD ["/bin/bash"]
