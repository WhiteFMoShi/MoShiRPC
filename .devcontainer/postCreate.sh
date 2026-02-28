#!/usr/bin/env bash
set -euo pipefail

workspace="${WORKSPACE_FOLDER:-/workspaces/MoShiRPC}"

# Copy host gitconfig to local file to avoid bind-mount permission edge cases.
if [ -r /tmp/host-gitconfig ]; then
  cp /tmp/host-gitconfig "${HOME}/.gitconfig" || true
  chmod 600 "${HOME}/.gitconfig" || true
fi

# Ensure commit message editing works even if host config/env points to a
# non-existent editor inside the container (for example: "editor").
editor_cmd_available() {
  local cmd="${1:-}"
  [ -n "${cmd}" ] || return 1
  command -v "${cmd%% *}" >/dev/null 2>&1
}

configured_editor="$(git config --global --get core.editor || true)"
if ! editor_cmd_available "${configured_editor}"; then
  if editor_cmd_available "code --wait"; then
    git config --global core.editor "code --wait" || true
  elif editor_cmd_available "nano"; then
    git config --global core.editor "nano" || true
  elif editor_cmd_available "vi"; then
    git config --global core.editor "vi" || true
  fi
fi

# Avoid "dubious ownership" when host UID/GID differs.
git config --global --add safe.directory "${workspace}" >/dev/null 2>&1 || true

# If host gitconfig enforces signed commits but gpg is unavailable in container,
# disable signing to keep normal commit flow working.
if [ "$(git config --global --get commit.gpgsign || echo false)" = "true" ]; then
  if ! command -v gpg >/dev/null 2>&1; then
    git config --global commit.gpgsign false || true
  fi
fi

# Keep SSH permissions strict so git over SSH works smoothly.
if [ -d "${HOME}/.ssh" ]; then
  chmod 700 "${HOME}/.ssh" || true
  find "${HOME}/.ssh" -type f -exec chmod 600 {} \; || true
fi

# If macOS compile_commands.json exists, clangd in Linux container may fail to
# resolve stdlib/include paths. Rename it to avoid accidental pickup.
while IFS= read -r -d '' db; do
  if grep -qE 'apple-macos|MacOSX\.sdk|CommandLineTools' "${db}"; then
    mv -f "${db}" "${db%.json}.macos.json"
  fi
done < <(find "${workspace}" -name compile_commands.json -print0 2>/dev/null || true)

# Ensure xmake is available without breaking container startup.
if ! command -v xmake >/dev/null 2>&1; then
  sudo apt-get update >/dev/null 2>&1 || true
  sudo apt-get install -y xmake >/dev/null 2>&1 || true
fi

if ! command -v xmake >/dev/null 2>&1; then
  for url in \
    "https://xmake.io/shget.text" \
    "https://fastly.jsdelivr.net/gh/xmake-io/xmake@dev/scripts/get.sh"; do
    if curl -fsSL "${url}" -o /tmp/xmake_install.sh; then
      bash /tmp/xmake_install.sh >/dev/null 2>&1 || true
      rm -f /tmp/xmake_install.sh
      break
    fi
  done

  if [ -x "${HOME}/.local/bin/xmake" ]; then
    sudo ln -sf "${HOME}/.local/bin/xmake" /usr/local/bin/xmake || true
  fi
fi

# Prefer a mirror repo for xmake package downloads.
if command -v xmake >/dev/null 2>&1; then
  xmake_repo_url="${XMAKE_REPO_URL:-https://gitee.com/tboox/xmake-repo.git}"
  xmake repo -r xmake-repo >/dev/null 2>&1 || true
  xmake repo -a xmake-repo "${xmake_repo_url}" master >/dev/null 2>&1 || true
  xmake repo -u >/dev/null 2>&1 || true

  if [ -n "${XMAKE_PROXY:-}" ]; then
    xmake g --proxy="${XMAKE_PROXY}" --proxy_hosts="github.com,*.github.com,raw.githubusercontent.com,*.xmake.io,gitee.com" >/dev/null 2>&1 || true
  fi
fi

# Quick sanity checks (don't fail the container if these are missing).
command -v clangd >/dev/null 2>&1 || echo "WARN: clangd not found in PATH"
command -v g++    >/dev/null 2>&1 || echo "WARN: g++ not found in PATH"
command -v cmake  >/dev/null 2>&1 || echo "WARN: cmake not found in PATH"
command -v xmake  >/dev/null 2>&1 || echo "WARN: xmake not found in PATH"
test -d "${CODEX_HOME:-}" || echo "WARN: CODEX_HOME is not mounted: ${CODEX_HOME:-unset}"

cat <<'MSG'
MoShiRPC devcontainer is ready.

Common commands:
  git config --list --show-origin | head
  cd rpc && xmake f -m debug -y && xmake -v
  cmake -S thread_pool -B thread_pool/build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  cmake --build thread_pool/build
MSG
