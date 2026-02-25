FROM ubuntu:25.10 AS base

RUN export debian_frontend=noninteractive \
 && apt-get update \
 && apt-get install -y software-properties-common \
 && add-apt-repository ppa:kisak/kisak-mesa \
 && apt-get update \
 && apt-get install -y vulkan-tools \
 && rm -rf /var/lib/apt/lists/*

# Build-time arguments for user/group IDs
ARG CMAKETAR=cmake-4.2.3-linux-x86_64.tar.gz
ARG CLANGTAR=LLVM-22.1.0-Linux-X64.tar.xz

# Install clang in non-interactive mode and clean up apt cache
RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get update \
 && apt-get install -y --no-install-recommends git ninja-build wget ca-certificates build-essential \
                                               curl zip unzip tar python3 pkg-config \
 && rm -rf /var/lib/apt/lists/*

# CMAKE
RUN cd \
 && wget https://github.com/Kitware/CMake/releases/download/v4.2.3/"${CMAKETAR}" \
 && tar -xvf "${CMAKETAR}" --strip-components=1 -C /usr/local \
 && rm "${CMAKETAR}"

# CLANG
RUN cd \
 && wget https://github.com/llvm/llvm-project/releases/download/llvmorg-22.1.0/"${CLANGTAR}" \
 && tar -xvf "${CLANGTAR}" --strip-components=1 -C /usr/local \
 && rm "${CLANGTAR}"

# vscode Section
FROM base AS vscode
ARG UID=1000
ARG GID=1000
ARG USER=vscode

# Install useful things for development
RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get update \
 && apt-get install -y --no-install-recommends sudo openssh-client lldb clangd \
 && rm -rf /var/lib/apt/lists/*

# Create group and user using provided IDs
RUN userdel -r ubuntu \
 && groupadd -g "${GID}" "${USER}" \
 && useradd -u "${UID}" -g "${GID}" -m -s /bin/bash "${USER}" \
 && echo "${USER}" ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/"${USER}" \
 && chmod 0440 /etc/sudoers.d/"${USER}"

USER ${USER}

# vcpkg
RUN cd "/home/${USER}" \
 && git clone https://github.com/microsoft/vcpkg.git \
 && cd vcpkg \
 && ./bootstrap-vcpkg.sh

ENV VCPKG_ROOT="/home/${USER}/vcpkg"
