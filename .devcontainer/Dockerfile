FROM ubuntu:resolute AS base

# Build-time arguments for user/group IDs
ARG CMAKETAR=cmake-4.2.3-linux-x86_64.tar.gz

# Install clang in non-interactive mode and clean up apt cache
RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get update \
 && apt-get install -y --no-install-recommends clang ninja-build wget ca-certificates \
 && rm -rf /var/lib/apt/lists/*

# CMAKE
RUN cd \
 && wget https://github.com/Kitware/CMake/releases/download/v4.2.3/"${CMAKETAR}" \
 && tar -xvf "${CMAKETAR}" --strip-components=1 -C /usr/local \
 && rm "${CMAKETAR}"

# Github section
FROM base AS github
ARG UID=1001
ARG GID=1001
ARG USER=github

# Create group and user using provided IDs
RUN userdel -r ubuntu \
 && groupadd -g "${GID}" "${USER}" \
 && useradd -u "${UID}" -g "${GID}" -m -s /bin/bash "${USER}"

USER ${USER}

# vscode Section
FROM base AS vscode
ARG UID=1000
ARG GID=1000
ARG USER=vscode

# Install useful things for development
RUN export DEBIAN_FRONTEND=noninteractive \
 && apt-get update \
 && apt-get install -y --no-install-recommends sudo git lldb clangd \
 && rm -rf /var/lib/apt/lists/*

# Create group and user using provided IDs
RUN userdel -r ubuntu \
 && groupadd -g "${GID}" "${USER}" \
 && useradd -u "${UID}" -g "${GID}" -m -s /bin/bash "${USER}" \
 && echo "${USER}" ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/"${USER}" \
 && chmod 0440 /etc/sudoers.d/"${USER}"

USER ${USER}
