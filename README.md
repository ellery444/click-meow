Click Meow  🐱

A tiny tray app for KDE Plasma Wayland that plays a random cat meow on every left click.

supporting environment

only KDE Plasma Wayland / KWin 6.7.5 / Qt 6.11.2 / Arch Linux
not for: Windows、macOS、GNOME or other Wayland othervKWin 

The program uses the KWin native interface, so it needs to be compiled on the target machine. After the KWin upgrade, you should exit the program, recompile it, and log in to the desktop again to clear the old QML plug-in cache.

you need

The development documents of CMake, C++20 compiler, Python 3, pkg-config, and Qt 6 Widgets / Multimedia / QML / Quick / DBus, KWin, KConfig, KCoreAddons, KWindowSystem, epoxy, Wayland, libdrm are required.

```sh
git clone https://github.com/ellery444/click-meow.git
cd click-meow
python3 install.py
./build/click-meow
```

The installation script will compile the program, link the KDE plug-in to the data directory of the current user, and add the application launcher entrance. After that, you can search for **tap, meow** in the launcher. No `sudo` is required for installation.

Please keep the cloned directory: the program reads the built-in sound source, icons and plug-ins from here. After moving the directory, it needs to be rebuilt and installed.

Use

Click the tray cat icon to open the settings, and the right-click menu can pause, audition or exit. After closing the setting window, the program continues to run on the tray. The volume and switch will be saved. The initial volume is 30%, and the login self-start is turned off by default.

"Add your own cat barking" will open `~/.local/share/click-meow/sounds/` (respect `XDG_DATA_HOME`). After inserting the **PCM WAV** file, it will automatically add random playback, and each file will not exceed 10 MiB. It is recommended to use short audio of less than two seconds; it does not support adding MP3 directly.

development & inspection

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

(un)install

- `~/.local/share/kwin/effects/clickmeow`：链接到项目的 `effect/`。
- `~/.local/share/applications/click-meow.desktop`：启动器入口。
- `~/.local/share/click-meow/sounds/`：用户音源。
- `~/.config/ellery/click-meow.conf`：偏好设置。
- `~/.config/autostart/click-meow.desktop`：勾选自启动后创建。

First turn off the self-start and exit from the tray, then delete the launcher file and plug-in symbol link, and finally delete the project directory. Custom sound sources and preferences can be kept on demand.

 License

Code: [GPL-3.0-or-later](LICENSE). Bundled sounds: CC0, Joseph SARDIN / BigSoundBank; see [SOUNDS.md](SOUNDS.md).
