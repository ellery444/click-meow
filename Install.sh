#!/usr/bin/env bash
set -u
meow_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
if ! command -v python3 >/dev/null 2>&1; then
    echo "需要 Python 3。请先通过系统的软件管理器安装 python3 / python。"
    exit 1
fi
python3 "$meow_dir/setup.py" "$@"
meow_status=$?
if [[ -t 0 && $# -eq 0 ]]; then
    read -r -p "按回车关闭安装程序…" || true
fi
exit "$meow_status"
