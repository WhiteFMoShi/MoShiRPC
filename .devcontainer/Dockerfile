FROM ubuntu:22.04

# 设置环境变量
ENV DEBIAN_FRONTEND=noninteractive
ENV LANG=zh_CN.UTF-8
ENV LC_ALL=zh_CN.UTF-8

# 安装语言包和基础工具
RUN apt update && apt install -y \
    locales \
    build-essential \
    g++ \
    gcc \
    clang \
    clangd \
    lldb \
    libclang-dev \
    python3 \
    vim \
    git \
    curl \
    sudo \
    cmake \
    ninja-build && \
    locale-gen zh_CN.UTF-8 && \
    rm -rf /var/lib/apt/lists/*

# 安装中文字体和GUI支持
RUN apt update && apt install -y \
    fonts-wqy-zenhei \
    fonts-wqy-microhei \
    libgtk-3-0 \
    libgbm1 \
    libnss3 \
    libxss1 \
    libasound2 && \
    rm -rf /var/lib/apt/lists/*

# 安装xmake
RUN curl -fsSL https://xmake.io/shget.text -o install.sh && \
    bash install.sh && \
    rm install.sh && \
    echo 'export PATH=~/.local/bin:$PATH' >> ~/.bashrc && \
    echo 'export PATH=~/.local/bin:$PATH' >> /root/.bashrc && \
    ln -sf ~/.local/bin/xmake /usr/local/bin/xmake

# 设置环境变量
ENV PATH=~/.local/bin:$PATH

# 设置工作目录
WORKDIR /workspace

CMD ["/bin/bash"]