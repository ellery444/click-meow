#!/usr/bin/env python3
"""Create a source installer archive from tracked files (including staged additions)."""
from pathlib import Path
import hashlib
import subprocess
import tarfile


def main():
    root = Path(__file__).resolve().parent
    version = (root / 'VERSION').read_text().strip()
    files = subprocess.check_output(['git', 'ls-files', '-z'], cwd=root).decode().split('\0')
    output = root / 'dist'
    output.mkdir(exist_ok=True)
    archive = output / 'click-meow-kde-linux.tar.gz'
    with tarfile.open(archive, 'w:gz') as tar:
        for name in sorted(filter(None, files)):
            path = root / name
            if path.is_symlink():
                raise RuntimeError(f'Refusing to package a symlink: {name}')
            info = tar.gettarinfo(str(path), arcname=f'click-meow-{version}/{name}')
            info.uid = info.gid = 0
            info.uname = info.gname = ''
            with path.open('rb') as source:
                tar.addfile(info, source)
    digest = hashlib.sha256(archive.read_bytes()).hexdigest()
    (output / 'SHA256SUMS').write_text(f'{digest}  {archive.name}\n')
    print(f'{archive} ({archive.stat().st_size:,} bytes)')


if __name__ == '__main__':
    main()
