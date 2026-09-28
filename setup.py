#!/usr/bin/env python3
"""Release installer: check dependencies, build in a durable user directory."""
import argparse
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys
import tempfile

ARCH_PACKAGES = ['base-devel', 'cmake', 'pkgconf', 'qt6-base', 'qt6-declarative',
                 'qt6-multimedia', 'kwin', 'kconfig', 'kcoreaddons',
                 'kwindowsystem', 'libepoxy', 'wayland', 'libdrm']
SOURCE_FILES = ['CMakeLists.txt', 'main.cpp', 'bridge.cpp', 'cat.svg',
                'click-meow.desktop.in', 'install.py', 'README.md', 'SOUNDS.md',
                'LICENSE', 'VERSION']


def arch_system():
    try:
        entries = dict(line.split('=', 1) for line in Path('/etc/os-release').read_text().splitlines() if '=' in line)
    except OSError:
        return False
    names = shlex.split(entries.get('ID', '')) + shlex.split(entries.get('ID_LIKE', ''))
    return any('arch' in name.split() for name in names)


def missing_tools():
    return [tool for tool in ['cmake', 'c++', 'make', 'pkg-config'] if not shutil.which(tool)]


def check_dependencies(check_only):
    if arch_system() and shutil.which('pacman'):
        result = subprocess.run(['pacman', '-T', *ARCH_PACKAGES], capture_output=True, text=True)
        if result.returncode not in (0, 127):
            raise RuntimeError('无法检查系统软件包：' + result.stderr)
        missing = result.stdout.split()
        if missing:
            command = ['sudo', 'pacman', '-S', '--needed', *missing]
            print('缺少依赖：' + ', '.join(missing), flush=True)
            print('安装命令：' + shlex.join(command), flush=True)
            if check_only:
                return False
            if not sys.stdin.isatty() or input('安装这些依赖？[y/N] ').strip().lower() not in ('y', 'yes'):
                raise RuntimeError('未安装依赖。完成上面的命令后，重新运行安装程序即可。')
            subprocess.run(command, check=True)
    missing = missing_tools()
    if missing:
        raise RuntimeError('缺少构建工具：' + ', '.join(missing) + '。请先通过软件管理器安装。')
    modules = ['Qt6Widgets', 'Qt6Multimedia', 'Qt6Quick', 'Qt6DBus', 'KF6CoreAddons',
               'KF6WindowSystem', 'epoxy', 'wayland-server', 'libdrm']
    missing = [m for m in modules if subprocess.run(['pkg-config', '--exists', m]).returncode]
    if missing:
        raise RuntimeError('缺少开发文件：' + ', '.join(missing) + '。请安装对应开发包，详见 README。')
    print('构建工具和 Qt/KDE 依赖检查通过。KWin 与 KConfig 头文件将在编译时检查。', flush=True)
    return True


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true', help='Only check environment and dependencies; do not install')
    args = parser.parse_args()
    if os.geteuid() == 0:
        raise RuntimeError('请以普通用户运行安装程序，不要使用 sudo。仅安装系统依赖时需要管理员权限。')
    if os.environ.get('XDG_SESSION_TYPE') != 'wayland' or 'KDE' not in os.environ.get('XDG_CURRENT_DESKTOP', '').upper():
        raise RuntimeError('请在 KDE Plasma 的 Wayland 桌面会话中运行。当前版本不支持其他桌面。')
    print('Click Meow · 点一下，喵一下\n检查 KDE Wayland 环境与构建依赖…', flush=True)
    if not check_dependencies(args.check):
        return 1
    if args.check:
        return 0
    source = Path(__file__).resolve().parent
    data_home = Path(os.environ.get('XDG_DATA_HOME', Path.home() / '.local/share')).resolve()
    versions = data_home / 'click-meow/versions'
    versions.mkdir(parents=True, exist_ok=True)
    version = (source / 'VERSION').read_text().strip()
    target = Path(tempfile.mkdtemp(prefix=version + '-', dir=versions))
    for name in SOURCE_FILES:
        shutil.copy2(source / name, target / name)
    for name in ['sounds', 'effect']:
        shutil.copytree(source / name, target / name,
                        ignore=shutil.ignore_patterns('*.so', '__pycache__'))
    log_path = target / 'install.log'
    print(f'正在编译并安装到：{target}\n首次安装可能需要几分钟。', flush=True)
    with log_path.open('w') as log:
        process = subprocess.Popen([sys.executable, str(target / 'install.py'), '--replace-existing'],
                                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        for line in process.stdout:
            print(line, end='', flush=True)
            log.write(line)
        result = process.wait()
    if result:
        raise RuntimeError(f'编译或安装失败，日志保存在 {log_path}。请根据日志补齐依赖；旧版本文件仍保留。')
    # Keep an existing login-start entry pointed at the new version.
    autostart = Path(os.environ.get('XDG_CONFIG_HOME', Path.home() / '.config')) / 'autostart/click-meow.desktop'
    if autostart.is_file():
        shutil.copyfile(target / 'build/click-meow.desktop', autostart)
    print('\n安装完成！在应用启动器搜索「点一下，喵一下」。\n现在可以删除下载的压缩包与解压目录。\n如果旧版正在运行，请先从托盘退出；升级插件后建议重新登录桌面。', flush=True)
    return 0


if __name__ == '__main__':
    try:
        sys.exit(main())
    except (RuntimeError, OSError, subprocess.CalledProcessError) as error:
        print(f'\n安装未完成：{error}', file=sys.stderr)
        sys.exit(1)
