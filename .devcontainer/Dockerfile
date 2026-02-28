FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID
ENV PATH=/home/${USERNAME}/.local/bin:/usr/local/bin:${PATH}

RUN apt-get update && apt-get install -y --no-install-recommends \
    locales \
    ca-certificates \
    curl \
    wget \
    git \
    openssh-client \
    sudo \
    bash \
    build-essential \
    gcc \
    g++ \
    make \
    pkg-config \
    cmake \
    ninja-build \
    clang \
    clangd \
    lldb \
    gdb \
    python3 \
    unzip \
    zip \
    tar \
    xz-utils \
    file \
    autoconf \
    automake \
    libtool \
    dnsutils \
    iputils-ping \
    net-tools \
    && rm -rf /var/lib/apt/lists/*

RUN groupadd --gid "${USER_GID}" "${USERNAME}" \
    && useradd --uid "${USER_UID}" --gid "${USER_GID}" -m "${USERNAME}" -s /bin/bash \
    && echo "${USERNAME} ALL=(root) NOPASSWD:ALL" > "/etc/sudoers.d/${USERNAME}" \
    && chmod 0440 "/etc/sudoers.d/${USERNAME}"

RUN locale-gen en_US.UTF-8 zh_CN.UTF-8

# Best effort preinstall xmake from apt. Final install is handled in postCreate.
RUN set -eux; \
    apt-get update; \
    apt-get install -y --no-install-recommends xmake || true; \
    rm -rf /var/lib/apt/lists/*; \
    if command -v xmake >/dev/null 2>&1; then xmake --version; fi

RUN mkdir -p /workspaces/MoShiRPC \
    && chown -R "${USERNAME}:${USERNAME}" /workspaces

WORKDIR /workspaces/MoShiRPC

USER ${USERNAME}

CMD ["/bin/bash"]
