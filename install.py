#!/usr/bin/env python3
"""Build and register Click Meow for the current user. No root required."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--no-build', action='store_true', help='Install an existing build')
    args = parser.parse_args()
    root = Path(__file__).resolve().parent
    data_home = Path(os.environ.get('XDG_DATA_HOME', Path.home() / '.local/share'))
    link = data_home / 'kwin/effects/clickmeow'
    if (link.exists() or link.is_symlink()) and link.resolve() != root / 'effect':
        raise SystemExit(f'Refusing to replace an unrelated existing path: {link}')
    if not args.no_build:
        subprocess.run(['cmake', '-S', str(root), '-B', str(root / 'build'),
                        '-DCMAKE_BUILD_TYPE=Release'], check=True)
        subprocess.run(['cmake', '--build', str(root / 'build'), '-j',
                        str(min(4, os.cpu_count() or 1))], check=True)
    required = [root / 'build/click-meow', root / 'build/click-meow.desktop',
                root / 'effect/contents/ui/MeowBridge/libmeowbridge.so']
    for file in required:
        if not file.is_file():
            raise SystemExit(f'Missing build output: {file}')
    link.parent.mkdir(parents=True, exist_ok=True)
    if not link.is_symlink():
        link.symlink_to(root / 'effect', target_is_directory=True)
    applications = data_home / 'applications'
    applications.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(root / 'build/click-meow.desktop', applications / 'click-meow.desktop')
    if shutil.which('update-desktop-database'):
        subprocess.run(['update-desktop-database', str(applications)], check=True)
    print('Installed. Search for “点一下，喵一下” in your application launcher.')
    print(f'Or run: {root / "build/click-meow"}')
    print('Keep this checkout in place: the application uses its sounds and effect plugin.')


if __name__ == '__main__':
    main()
