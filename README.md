Click Meow  🐱

A tiny tray app for KDE Plasma Wayland and Windows that plays a random cat meow on every left click.

## Windows portable version

Windows 10/11 x64 support is available as a portable build. It uses Windows Raw Input, with no KDE dependency or administrator rights needed. Extract the **entire ZIP**, then run `click-meow.exe`. The tray menu, random meows, volume, custom WAV files and optional login startup work through the same interface.

The Windows package is named **click-meow-windows-x64.zip**; published packages will appear on the [Releases page](https://github.com/ellery444/click-meow/releases). Automated builds are also available under [Windows portable build → Artifacts](https://github.com/ellery444/click-meow/actions/workflows/windows.yml) after a successful run (GitHub sign-in required). Instructions are in [README-Windows.txt](windows/README-Windows.txt).

The Windows portable build has passed automated build and packaging checks, and real-PC testing has been reported successful. This does not imply testing on every Windows version or mouse/touchpad model.

Windows builds use Qt 6.8.3 and MSVC 2022. CMake selects the Windows backend automatically; `windows/package.ps1` collects Qt and MSVC runtime DLLs, tests the portable directory, and produces the ZIP. Do not move only the EXE out of its folder. Disable and re-enable login startup after moving the whole folder.

## Download & install / 下载安装

**[⬇ Download the KDE Linux installer](https://github.com/ellery444/click-meow/releases/latest/download/click-meow-kde-linux.tar.gz)** · [Release notes](https://github.com/ellery444/click-meow/releases/latest)

Extract the archive, open a terminal in the extracted folder, and run:

```sh
bash Install.sh
```

The installer checks KDE Wayland and build dependencies, copies the app into your user data directory, compiles it, and adds an application launcher entry. Once installed, search for **Click Meow / 点一下，喵一下**. You can delete the downloaded archive and extracted folder afterwards.

This is a **guided source installer**, not a universal prebuilt binary. On Arch Linux, it offers to install missing dependencies with your approval; other distributions require their development packages to be installed manually. Tested on Arch Linux / KWin 6.7.5; other KWin versions remain untested. Run `bash Install.sh --check` for a check without installation. Build logs are saved under `~/.local/share/click-meow/versions/<version-directory>/install.log`.

中文：下载解压，在解压目录运行 `bash Install.sh`，按提示安装依赖并等待编译。完成后可删除下载包和解压目录，再从应用启动器打开“点一下，喵一下”。仅适用于兼容的 KDE Wayland 环境。

**Linux supporting environment**

only KDE Plasma Wayland / KWin 6.7.5 / Qt 6.11.2 / Arch Linux
The Linux installer does not support Windows, macOS, GNOME or other Wayland compositors. For Windows, use the separate portable build above.

The program uses the KWin native interface, so it needs to be compiled on the target machine. After the KWin upgrade, you should exit the program, recompile it, and log in to the desktop again to clear the old QML plug-in cache.


**Build from Git (developers)**

The development documents of CMake, C++20 compiler, Python 3, pkg-config, and Qt 6 Widgets / Multimedia / QML / Quick / DBus, KWin, KConfig, KCoreAddons, KWindowSystem, epoxy, Wayland, libdrm are required.

```sh
git clone https://github.com/ellery444/click-meow.git
cd click-meow
python3 install.py
./build/click-meow
```

The installation script will compile the program, link the KDE plug-in to the data directory of the current user, and add the application launcher entrance. After that, you can search for **tap, meow** in the launcher. No `sudo` is required for installation.

Please keep the cloned directory: the program reads the built-in sound source, icons and plug-ins from here. After moving the directory, it needs to be rebuilt and installed.

**Use**

Click the tray cat icon to open the settings, and the right-click menu can pause, audition or exit. After closing the setting window, the program continues to run on the tray. The volume and switch will be saved. The initial volume is 30%, and the login self-start is turned off by default.

"Add your own cat barking" will open `~/.local/share/click-meow/sounds/` (respect `XDG_DATA_HOME`). After inserting the **PCM WAV** file, it will automatically add random playback, and each file will not exceed 10 MiB. It is recommended to use short audio of less than two seconds; it does not support adding MP3 directly.

**development & inspection**

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
python3 install.py --no-build
./build/click-meow

qdbus6 local.clickmeow.App /ClickMeow local.clickmeow.App.Status
python3 tests/smoke.py
```

The integrated check will briefly pause the listening, verify that it will not play after suspension, the audition will not be repeated continuously, and then restore the original switch state. Two auditions will be played.

Verified: real machine global click event arrival, 11 segments of audio decoding ready, pause and resume, continuous random playback without repetition. Whether the sound is output through the current speaker is still subject to the actual listening experience.

If the setting shows "Mouse listening is not connected", check whether the plug-in symbol link exists, rerun the installation script, and check the `clickmeow` / `MeowBridge` error in the KWin log. After upgrading KWin, please recompile and log in again.

**(un)install**

- `~/.local/share/kwin/effects/clickmeow`：链接到项目的 `effect/`。
- `~/.local/share/applications/click-meow.desktop`：启动器入口。
- `~/.local/share/click-meow/sounds/`：用户音源。
- `~/.local/share/click-meow/versions/`: installed release files and build logs; old versions are retained on upgrade.
- `~/.config/ellery/click-meow.conf`：偏好设置。
- `~/.config/autostart/click-meow.desktop`：勾选自启动后创建。

First turn off the self-start and exit from the tray, then delete the launcher file and plug-in symbol link, and finally delete the project directory or the installed `versions/` directory. Custom sound sources and preferences can be kept on demand.

 License

Code: [GPL-3.0-or-later](LICENSE). Bundled sounds: CC0, Joseph SARDIN / BigSoundBank; see [SOUNDS.md](SOUNDS.md).
