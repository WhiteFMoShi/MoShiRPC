#!/usr/bin/env bash
set -euo pipefail

workspace="${WORKSPACE_FOLDER:-/workspaces/MoShiRPC}"

# Avoid "dubious ownership" when the host UID/GID differs.
git config --global --add safe.directory "${workspace}" >/dev/null 2>&1 || true

# If you previously generated `compile_commands.json` on macOS, clangd inside Linux
# will try to use it and then fail to find the standard library/sysroot.
# Rename those databases so clangd falls back (or you can regenerate on Linux).
while IFS= read -r -d '' db; do
  if grep -qE 'apple-macos|MacOSX\.sdk|CommandLineTools' "${db}"; then
    mv -f "${db}" "${db%.json}.macos.json"
  fi
done < <(find "${workspace}" -name compile_commands.json -print0 2>/dev/null || true)

# Quick sanity checks (don't fail the container if these are missing).
command -v clangd >/dev/null 2>&1 || echo "WARN: clangd not found in PATH"
command -v cmake  >/dev/null 2>&1 || echo "WARN: cmake not found in PATH"
command -v xmake  >/dev/null 2>&1 || echo "WARN: xmake not found in PATH"

cat <<'EOF'
MoShiRPC devcontainer is ready.

Common commands:
  cd rpc && xmake f -m debug && xmake
  cmake -S thread_pool -B thread_pool/build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  cmake --build thread_pool/build
EOF
