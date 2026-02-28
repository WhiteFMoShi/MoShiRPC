# MoShiRPC Dev Container (Clean Setup)

目标：

1. Git 提交/推送尽量无缝
2. 容器内可直接使用 Codex 插件与配置
3. 容器网络可用于 `apt` 和 `xmake`

## 配置说明

- 基础镜像：Ubuntu 22.04
- 编译器：`g++`（同时保留 `clangd` 作为 LSP）
- 已安装工具：`git`、`ssh`、`cmake`、`ninja`、`xmake`、`gdb/lldb`
- 挂载项：
  - `~/.gitconfig` -> `/tmp/host-gitconfig`（容器启动后复制到 `/home/vscode/.gitconfig`）
  - `~/.ssh` -> `/home/vscode/.ssh`
  - `~/.codex` -> `/home/vscode/.codex`
- 代理透传：`HTTP_PROXY/HTTPS_PROXY/ALL_PROXY/NO_PROXY`（大小写都透传）

## 使用步骤

1. VS Code 打开项目根目录
2. 执行 `Dev Containers: Rebuild Container`
3. 容器中验证：

```bash
git config --list --show-origin | head
echo "$CODEX_HOME" && ls -la "$CODEX_HOME"
g++ --version
xmake --version
```

## 构建命令

```bash
cd rpc
xmake f -m debug -y
xmake -v
```

```bash
cmake -S thread_pool -B thread_pool/build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build thread_pool/build
```

## Git 提交异常排查

- 如果报 `unable to access '/home/vscode/.gitconfig': Permission denied`，重建容器后再试（新配置已修复该问题）。
- 如果报 `gpg failed to sign the data`，容器会在无 `gpg` 时自动关闭 `commit.gpgsign`。

## 网络相关建议

- 该配置不会强制写死 DNS，避免和你本机网络冲突。
- 如需代理，先在 mac 上导出代理环境变量，再重建容器。
- `postCreate.sh` 会将 xmake 仓库切到镜像地址（默认 `gitee`）以提高依赖下载成功率。
