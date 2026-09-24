# Click Meow · 点一下，喵一下 🐱

**每点一下鼠标左键，就随机喵一声。**

A tiny tray app for KDE Plasma Wayland that plays a random cat meow on every left click.

- 自带 11 段真实猫叫，每段约 0.37–0.75 秒，连续两次不选同一段。
- 在其他应用窗口中点击也能触发，鼠标事件正常传递。
- 托盘开关、音量调节、试听、可选登录自启动。
- 支持添加自己的 WAV 音源；快速点击最多叠加四个声音。
- 锁屏时不触发；暂停或正常退出时卸载监听。
- 运行时完全离线，无需 root、input 用户组或 evdev 权限。

## 支持环境

目前仅在 **KDE Plasma Wayland / KWin 6.7.5 / Qt 6.11.2 / Arch Linux** 验证。
这不是 Windows、macOS、GNOME 或通用 Wayland 程序。其他 KWin 版本尚未测试。

程序使用 KWin 原生接口，因此需要在目标机器上编译。KWin 升级后应退出程序、重新编译，并重新登录桌面以清除旧的 QML 插件缓存。

## 安装

需要 CMake、C++20 编译器、Python 3、pkg-config，以及 Qt 6 Widgets / Multimedia / QML / Quick / DBus、KWin、KConfig、KCoreAddons、KWindowSystem、epoxy、Wayland、libdrm 的开发文件。

```sh
git clone https://github.com/ellery444/click-meow.git
cd click-meow
python3 install.py
./build/click-meow
```

安装脚本会编译程序，将 KDE 插件链接到当前用户的数据目录，并添加应用启动器入口。之后可在启动器中搜索 **点一下，喵一下**。安装不需要 `sudo`。

请保留克隆的目录：程序从这里读取内置音源、图标和插件。移动目录后需重新构建和安装。

## 使用

点击托盘猫咪图标打开设置，右键菜单可暂停、试听或退出。关闭设置窗口后，程序继续在托盘运行。音量与开关会保存，初始音量为 30%，登录自启动默认关闭。

“添加自己的猫叫”会打开 `~/.local/share/click-meow/sounds/`（尊重 `XDG_DATA_HOME`）。放入 **PCM WAV** 文件后自动加入随机播放，每个文件不超过 10 MiB。建议使用不到两秒的短音频；不支持直接添加 MP3。

## 工作原理

```text
KWin 左键按下事件 → 小型 QML/C++ 插件 → D-Bus → Qt 托盘程序 → 随机猫叫
```

`bridge.cpp` 通过 `EffectsHandler::mouseChanged` 观察左键从松开到按下的变化，只发送一个不含坐标的 D-Bus 信号。插件不显示窗口，不拦截输入。松开、拖动中的移动、右键和滚轮不会产生额外猫叫。

声音由独立进程中的 `QSoundEffect` 预加载和播放。所有音源是独立录音，未通过变调凑数。来源、授权与加工说明见 [SOUNDS.md](SOUNDS.md)。

## 开发与检查

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
python3 install.py --no-build
./build/click-meow

# 在程序运行时检查状态
qdbus6 local.clickmeow.App /ClickMeow local.clickmeow.App.Status
python3 tests/smoke.py
```

集成检查会短暂暂停监听，验证暂停后不播放、试听不连续重复，然后恢复原开关状态。会播放两声试听。

已验证：实机全局点击事件到达、11 段音频解码就绪、暂停和恢复、连续随机播放不重复。声音是否经当前扬声器输出仍以实际听感为准。

如果设置显示“鼠标监听未连接”，检查插件符号链接是否存在、重新运行安装脚本，并查看 KWin 日志中的 `clickmeow` / `MeowBridge` 错误。升级 KWin 后请重新编译并重新登录。

## 安装文件与卸载

- `~/.local/share/kwin/effects/clickmeow`：链接到项目的 `effect/`。
- `~/.local/share/applications/click-meow.desktop`：启动器入口。
- `~/.local/share/click-meow/sounds/`：用户音源。
- `~/.config/ellery/click-meow.conf`：偏好设置。
- `~/.config/autostart/click-meow.desktop`：勾选自启动后创建。

先关闭自启动并从托盘退出，再删除启动器文件和插件符号链接，最后删除项目目录。自定义音源和偏好设置可按需保留。

## License

Code: [GPL-3.0-or-later](LICENSE). Bundled sounds: CC0, Joseph SARDIN / BigSoundBank; see [SOUNDS.md](SOUNDS.md).
