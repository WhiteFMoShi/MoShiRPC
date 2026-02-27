FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    git \
    sudo \
    bash \
    build-essential \
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
    autoconf \
    automake \
    libtool \
    && rm -rf /var/lib/apt/lists/*

RUN groupadd --gid "${USER_GID}" "${USERNAME}" \
    && useradd --uid "${USER_UID}" --gid "${USER_GID}" -m "${USERNAME}" -s /bin/bash \
    && echo "${USERNAME} ALL=(root) NOPASSWD:ALL" > "/etc/sudoers.d/${USERNAME}" \
    && chmod 0440 "/etc/sudoers.d/${USERNAME}"

# Install xmake (try apt first; fallback to upstream installer).
ARG XMAKE_INSTALL_URL=https://xmake.io/shget.text
RUN set -eux; \
    apt-get update; \
    apt-get install -y --no-install-recommends xmake || true; \
    rm -rf /var/lib/apt/lists/*; \
    if ! command -v xmake >/dev/null 2>&1; then \
        curl -fsSL "${XMAKE_INSTALL_URL}" -o /tmp/xmake_install.sh; \
        bash /tmp/xmake_install.sh; \
        rm -f /tmp/xmake_install.sh; \
        ln -sf /root/.local/bin/xmake /usr/local/bin/xmake; \
    fi

RUN mkdir -p /workspaces/MoShiRPC \
    && chown -R "${USERNAME}:${USERNAME}" /workspaces

WORKDIR /workspaces/MoShiRPC

USER ${USERNAME}

CMD ["/bin/bash"]
