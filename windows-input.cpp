#include "windows-input.h"
#include <QCoreApplication>

WindowsInput::WindowsInput(QObject *parent) : QObject(parent) {
    QCoreApplication::instance()->installNativeEventFilter(this);
}
WindowsInput::~WindowsInput() {
    stop();
    QCoreApplication::instance()->removeNativeEventFilter(this);
}
bool WindowsInput::start() {
    if (active) return true;
    // INPUTSINK observes mouse input while other apps have focus. No grab,
    // keyboard registration, suppression of legacy events, or administrator rights.
    RAWINPUTDEVICE device{0x01, 0x02, RIDEV_INPUTSINK, reinterpret_cast<HWND>(target.winId())};
    active = RegisterRawInputDevices(&device, 1, sizeof(device));
    error = active ? QString() : QString("Windows 鼠标监听注册失败（错误 %1）。").arg(GetLastError());
    return active;
}
void WindowsInput::stop() {
    if (!active) return;
    active = false;
    RAWINPUTDEVICE device{0x01, 0x02, RIDEV_REMOVE, nullptr};
    RegisterRawInputDevices(&device, 1, sizeof(device));
}
bool WindowsInput::isLeftPress(const RAWINPUT &input) {
    return input.header.dwType == RIM_TYPEMOUSE
        && (input.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN);
}
bool WindowsInput::nativeEventFilter(const QByteArray &, void *message, qintptr *) {
    const auto msg = static_cast<MSG *>(message);
    if (active && msg->message == WM_INPUT
        && msg->hwnd == reinterpret_cast<HWND>(target.winId())) {
        RAWINPUT input{};
        UINT size = sizeof(input);
        const UINT count = GetRawInputData(reinterpret_cast<HRAWINPUT>(msg->lParam),
                                          RID_INPUT, &input, &size, sizeof(RAWINPUTHEADER));
        if (count != UINT(-1) && count >= sizeof(RAWINPUTHEADER) + sizeof(RAWMOUSE)
            && isLeftPress(input)) emit leftPressed();
    }
    // Let Qt/DefWindowProc perform normal WM_INPUT cleanup and event delivery.
    return false;
}
