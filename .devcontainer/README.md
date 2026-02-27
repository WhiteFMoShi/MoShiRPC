# MoShiRPC Dev Container

这个项目依赖 Linux 的 `epoll`，在 macOS 上建议用 VS Code Dev Containers 跑一个 Ubuntu 22.04 的开发环境。

## 使用方式

1. 安装 VS Code 扩展：`Dev Containers`（ms-vscode-remote.remote-containers）
2. 在项目根目录打开 VS Code
3. `Dev Containers: Reopen in Container`（或 `Rebuild Container`）

## 常用构建命令

RPC（xmake）：

```bash
cd rpc
xmake f -m debug
xmake
```

线程池（CMake）：

```bash
cmake -S thread_pool -B thread_pool/build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build thread_pool/build
```

## 常见问题

- 容器起不来 / `devcontainer.json` 报错：确认 `.devcontainer/devcontainer.json` 是合法 JSON（不能有多余逗号）。
- `clangd` 找不到标准库头文件（比如 `<vector>`）：通常是仓库里残留了你在 macOS 上生成的 `compile_commands.json`，里面带 `-isysroot ...MacOSX.sdk`，在 Linux 容器里会直接失效。现在 `.devcontainer/postCreate.sh` 会自动把这类文件改名为 `compile_commands.macos.json`；然后你在容器里重新生成一份 Linux 的编译数据库即可（例如 `cd rpc && xmake project -k compile_commands`）。
- 网络相关失败（`apt`/`curl`/`xmake` 下载失败）：先不要在 `runArgs` 里强行指定 DNS；必要时再按你当前网络环境配置 Docker 的 DNS/代理。
- 需要用 SSH 拉私有仓库：
  - 建议通过 VS Code 的 Git/凭据管理走 HTTPS；或者你自行在 `devcontainer.json` 加 `mounts` 把宿主机 `~/.ssh` 挂进去。
