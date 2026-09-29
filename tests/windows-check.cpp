#include "../windows-input.h"
#include <QApplication>
#include <QAudioDecoder>
#include <QDir>
#include <QEventLoop>
#include <QIcon>
#include <QTimer>
#include <QUrl>
#include <QDebug>

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    if (argc != 2) return 1;
    RAWINPUT packet{};
    packet.header.dwType = RIM_TYPEMOUSE;
    for (USHORT flags : {USHORT(0), USHORT(RI_MOUSE_LEFT_BUTTON_UP), USHORT(RI_MOUSE_RIGHT_BUTTON_DOWN), USHORT(RI_MOUSE_WHEEL)}) {
        packet.data.mouse.usButtonFlags = flags;
        if (WindowsInput::isLeftPress(packet)) return 2;
    }
    packet.data.mouse.usButtonFlags = RI_MOUSE_LEFT_BUTTON_DOWN;
    if (!WindowsInput::isLeftPress(packet)) return 3;
    packet.header.dwType = RIM_TYPEKEYBOARD;
    if (WindowsInput::isLeftPress(packet)) return 4;
    WindowsInput input;
    if (!input.start() || !input.isActive()) { qCritical() << input.errorString(); return 5; }
    input.stop();
    if (input.isActive() || !input.start()) return 6;
    input.stop();
    const QString root = QString::fromLocal8Bit(argv[1]);
    if (QIcon(root + "/cat.svg").pixmap(32, 32).isNull()) return 7;
    const auto sounds = QDir(root + "/sounds").entryInfoList({"*.wav"}, QDir::Files);
    if (sounds.size() != 11) return 8;
    for (const auto &file : sounds) {
        QAudioDecoder decoder;
        QEventLoop loop;
        QTimer timer;
        timer.setSingleShot(true);
        bool finished = false;
        bool failed = false;
        int buffers = 0;
        QObject::connect(&decoder, &QAudioDecoder::bufferReady, &loop, [&] {
            if (decoder.read().isValid()) ++buffers;
        });
        QObject::connect(&decoder, &QAudioDecoder::finished, &loop, [&] { finished = true; loop.quit(); });
        QObject::connect(&decoder, qOverload<QAudioDecoder::Error>(&QAudioDecoder::error), &loop,
                         [&](auto) { failed = true; loop.quit(); });
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        decoder.setSource(QUrl::fromLocalFile(file.absoluteFilePath()));
        timer.start(10000);
        decoder.start();
        loop.exec();
        if (!finished || failed || buffers == 0) {
            qCritical() << "Decode failed" << file.fileName() << decoder.errorString();
            return 9;
        }
    }
    qInfo() << "PASS: left-button filtering, native registration/pause/resume, SVG icon, 11 WAV decodes";
    return 0;
}
