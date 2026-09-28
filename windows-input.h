#pragma once
#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QWindow>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

class WindowsInput : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
public:
    explicit WindowsInput(QObject *parent = nullptr);
    ~WindowsInput() override;
    bool start();
    void stop();
    bool isActive() const { return active; }
    QString errorString() const { return error; }
    bool nativeEventFilter(const QByteArray &, void *message, qintptr *) override;
    static bool isLeftPress(const RAWINPUT &input);
signals:
    void leftPressed();
private:
    QWindow target;
    bool active = false;
    QString error;
};
