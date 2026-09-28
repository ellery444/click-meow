#include <QApplication>
#include <QCheckBox>
#ifdef Q_OS_WIN
#include "windows-input.h"
#include <QLocalServer>
#include <QLocalSocket>
#include <QCryptographicHash>
#else
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#endif
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileSystemWatcher>
#include <QFormLayout>
#include <QIcon>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSettings>
#include <QSlider>
#include <QSoundEffect>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTimer>
#include <algorithm>

static QString root;
static const QString service = QStringLiteral("local.clickmeow.App");

class MeowApp : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "local.clickmeow.App")
#ifdef Q_OS_WIN
    WindowsInput input;
#endif
    QSettings settings;
    QSystemTrayIcon tray;
    QMenu menu;
    QDialog dialog;
    QLabel *stateLabel;
    QLabel *volumeLabel;
    QCheckBox *enabledBox;
    QAction *enabledAction;
    QList<QSoundEffect *> sounds;
    QList<QSoundEffect *> playing;
    QFileSystemWatcher watcher;
    QString customDir;
    QString lastSource;
    QString error;
    bool enabled = true;
    bool listenerLoaded = false;
    int volume = 30;
    int lastIndex = -1;
    int clicks = 0;
    int plays = 0;

    #ifndef Q_OS_WIN
    QDBusMessage effectCall(const QString &method) {
        QDBusInterface effects("org.kde.KWin", "/Effects", "org.kde.kwin.Effects");
        effects.setTimeout(2000);
        return effects.call(method, "clickmeow");
    }
    #endif
    void updateState() {
        int ready = 0;
        for (auto sound : sounds) ready += sound->status() == QSoundEffect::Ready;
        QString state = !enabled ? "已暂停" : listenerLoaded ? "左键点击就会喵一声" : "鼠标监听未连接";
        if (enabled && ready == 0) state = "音源尚未就绪";
        stateLabel->setText(QString("%1\n%2 段音源已就绪%3").arg(state).arg(ready)
                            .arg(error.isEmpty() ? "" : "\n" + error));
        tray.setToolTip("点一下，喵一下 · " + state);
    }
    void refreshListener() {
        if (!enabled) return;
#ifdef Q_OS_WIN
        listenerLoaded = input.start();
        error = input.errorString();
#else
        QDBusReply<bool> loaded = effectCall("isEffectLoaded");
        if (loaded.isValid() && loaded.value()) {
            listenerLoaded = true;
            error.clear();
        } else {
            QDBusReply<bool> reply = effectCall("loadEffect");
            listenerLoaded = reply.isValid() && reply.value();
            error = listenerLoaded ? "" : "KDE 点击插件未加载；请查看 README 的修复说明。";
        }
#endif
        updateState();
    }
    void stopListener() {
#ifdef Q_OS_WIN
        input.stop();
#else
        effectCall("unloadEffect");
#endif
        listenerLoaded = false;
    }
    void setEnabled(bool value) {
        enabled = value;
        settings.setValue("enabled", value);
        enabledBox->setChecked(value);
        enabledAction->setChecked(value);
        if (value) refreshListener();
        else {
            stopListener();
            for (auto s : sounds) s->stop();
            playing.clear();
            error.clear();
            updateState();
        }
    }
    void loadSounds() {
        playing.clear();
        qDeleteAll(sounds);
        sounds.clear();
        lastIndex = -1;
        for (const auto &dir : {root + "/sounds", customDir}) {
            for (const auto &file : QDir(dir).entryInfoList({"*.wav", "*.WAV"}, QDir::Files, QDir::Name)) {
                if (file.size() > 10 * 1024 * 1024) continue;
                auto sound = new QSoundEffect(this);
                sound->setVolume(volume / 100.0);
                connect(sound, &QSoundEffect::statusChanged, this, [this, sound] {
                    if (sound->status() == QSoundEffect::Error)
                        qWarning() << "Cannot decode" << sound->source();
                    updateState();
                });
                sounds.append(sound);
                sound->setSource(QUrl::fromLocalFile(file.absoluteFilePath()));
            }
        }
        updateState();
    }
    void play() {
        QList<int> available;
        for (int i = 0; i < sounds.size(); ++i)
            if (sounds[i]->status() == QSoundEffect::Ready) available.append(i);
        if (available.isEmpty()) return;
        if (available.size() > 1) available.removeAll(lastIndex);
        int index = available.at(QRandomGenerator::global()->bounded(int(available.size())));
        auto sound = sounds[index];
        playing.removeIf([](auto s) { return !s->isPlaying(); });
        playing.removeAll(sound);
        // A rapid burst never accumulates more than four voices.
        while (playing.size() >= 4) playing.takeFirst()->stop();
        sound->stop();
        sound->play();
        playing.append(sound);
        lastIndex = index;
        lastSource = sound->source().fileName();
        ++plays;
    }
public:
    explicit MeowApp() : settings("ellery", "click-meow"), tray(QIcon(root + "/cat.svg")) {
        enabled = settings.value("enabled", true).toBool();
        volume = std::clamp(settings.value("volume", 30).toInt(), 0, 100);
        customDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/click-meow/sounds";
        QDir().mkpath(customDir);
        dialog.setWindowTitle("点一下，喵一下");
        dialog.setWindowIcon(QIcon(root + "/cat.svg"));
        dialog.setMinimumWidth(370);
        auto layout = new QFormLayout(&dialog);
        auto title = new QLabel("<h2>点一下，喵一下 🐱</h2>");
        layout->addRow(title);
        stateLabel = new QLabel;
        stateLabel->setWordWrap(true);
        layout->addRow(stateLabel);
        enabledBox = new QCheckBox("开启随机猫叫");
        enabledBox->setChecked(enabled);
        layout->addRow(enabledBox);
        auto slider = new QSlider(Qt::Horizontal);
        slider->setRange(0, 100);
        slider->setValue(volume);
        volumeLabel = new QLabel(QString("音量 %1%").arg(volume));
        layout->addRow(volumeLabel, slider);
        auto preview = new QPushButton("随机喵一声 · 试听");
        layout->addRow(preview);
        auto folder = new QPushButton("添加自己的猫叫…");
        layout->addRow(folder);
        auto hint = new QLabel("将 WAV 音频放进打开的文件夹，即可加入随机播放。\n关闭此窗口后，仍可从托盘里的猫咪图标暂停或退出。");
        hint->setWordWrap(true);
        layout->addRow(hint);
        auto autostart = new QCheckBox("登录桌面时自动启动");
#ifdef Q_OS_WIN
        const auto autostartPath = QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run");
        autostart->setChecked(QSettings(autostartPath, QSettings::NativeFormat).contains("ClickMeow"));
#else
        const auto autostartPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/autostart/click-meow.desktop";
        autostart->setChecked(QFile::exists(autostartPath));
#endif
        layout->addRow(autostart);
        enabledAction = menu.addAction("开启随机猫叫");
        enabledAction->setCheckable(true);
        enabledAction->setChecked(enabled);
        menu.addAction("随机喵一声", this, &MeowApp::PlayTest);
        menu.addAction("设置…", this, &MeowApp::Show);
        menu.addAction("添加音源…", this, [this] { QDesktopServices::openUrl(QUrl::fromLocalFile(customDir)); });
        menu.addSeparator();
        menu.addAction("退出", qApp, &QApplication::quit);
        tray.setContextMenu(&menu);
        connect(&tray, &QSystemTrayIcon::activated, this, [this](auto reason) {
            if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) Show();
        });
        connect(enabledBox, &QCheckBox::clicked, this, &MeowApp::setEnabled);
        connect(enabledAction, &QAction::triggered, this, &MeowApp::setEnabled);
        connect(preview, &QPushButton::clicked, this, &MeowApp::PlayTest);
        connect(folder, &QPushButton::clicked, this, [this] { QDesktopServices::openUrl(QUrl::fromLocalFile(customDir)); });
        connect(slider, &QSlider::valueChanged, this, [this](int value) {
            volume = value;
            settings.setValue("volume", value);
            volumeLabel->setText(QString("音量 %1%").arg(value));
            for (auto sound : sounds) sound->setVolume(value / 100.0);
        });
        connect(autostart, &QCheckBox::clicked, this, [this, autostart, autostartPath](bool value) {
            bool ok;
#ifdef Q_OS_WIN
            QSettings startup(autostartPath, QSettings::NativeFormat);
            if (value) startup.setValue("ClickMeow", "\"" + QDir::toNativeSeparators(QCoreApplication::applicationFilePath()) + "\"");
            else startup.remove("ClickMeow");
            startup.sync();
            ok = startup.status() == QSettings::NoError;
#else
            if (value) {
                QDir().mkpath(QFileInfo(autostartPath).absolutePath());
                ok = QFile::copy(QStringLiteral(MEOW_DESKTOP_PATH), autostartPath);
            } else ok = QFile::remove(autostartPath);
#endif
            if (!ok) {
#ifdef Q_OS_WIN
                autostart->setChecked(startup.contains("ClickMeow"));
#else
                autostart->setChecked(QFile::exists(autostartPath));
#endif
                QMessageBox::warning(&dialog, "未能修改自启动", "请检查自启动文件的权限。");
            }
        });
#ifdef Q_OS_WIN
        connect(&input, &WindowsInput::leftPressed, this, &MeowApp::LeftPressed, Qt::QueuedConnection);
#else
        auto bus = QDBusConnection::sessionBus();
        bus.registerObject("/ClickMeow", this, QDBusConnection::ExportAllSlots);
        bus.connect("org.kde.KWin", "/ClickMeow", "local.clickmeow.Input", "LeftPressed", this, SLOT(LeftPressed()));
#endif
        connect(qApp, &QCoreApplication::aboutToQuit, this, &MeowApp::stopListener);
        loadSounds();
        watcher.addPath(customDir);
        auto debounce = new QTimer(this);
        debounce->setSingleShot(true);
        debounce->setInterval(700);
        connect(&watcher, &QFileSystemWatcher::directoryChanged, debounce, qOverload<>(&QTimer::start));
        connect(debounce, &QTimer::timeout, this, &MeowApp::loadSounds);
        auto health = new QTimer(this);
        health->setInterval(10000);
        connect(health, &QTimer::timeout, this, &MeowApp::refreshListener);
        health->start();
        tray.show();
        QTimer::singleShot(0, this, &MeowApp::refreshListener);
        if (!settings.value("seen", false).toBool() || qApp->arguments().contains("--settings")) Show();
        settings.setValue("seen", true);
    }
public slots:
    void Show() { dialog.show(); dialog.raise(); dialog.activateWindow(); }
    void PlayTest() { play(); }
    void SetEnabled(bool value) { setEnabled(value); }
    void LeftPressed() { if (enabled) { ++clicks; play(); } }
    void Quit() { qApp->quit(); }
    QString Status() {
        int ready = 0;
        for (auto sound : sounds) ready += sound->status() == QSoundEffect::Ready;
        return QString::fromUtf8(QJsonDocument(QJsonObject{
            {"enabled", enabled}, {"listenerLoaded", listenerLoaded}, {"sounds", int(sounds.size())},
            {"ready", ready}, {"clicks", clicks}, {"plays", plays}, {"last", lastSource},
            {"volume", volume}, {"error", error}
        }).toJson(QJsonDocument::Compact));
    }
};

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    app.setApplicationName("click-meow");
    app.setQuitOnLastWindowClosed(false);
#ifdef Q_OS_WIN
    root = QCoreApplication::applicationDirPath();
    const QString userPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    const QString socketName = "click-meow-" + QString::fromLatin1(QCryptographicHash::hash(userPath.toUtf8(), QCryptographicHash::Sha256).toHex().left(16));
    QLocalSocket peer;
    peer.connectToServer(socketName);
    if (peer.waitForConnected(500)) {
        peer.write("show");
        peer.waitForBytesWritten(500);
        return 0;
    }
    QLocalServer server;
    server.setSocketOptions(QLocalServer::UserAccessOption);
    if (!server.listen(socketName)) {
        QMessageBox::warning(nullptr, "Click Meow", "程序已在运行，或无法创建本地通信端点。");
        return 1;
    }
#else
    root = QStringLiteral(MEOW_ROOT);
    auto bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) return 1;
    if (!bus.registerService(service)) {
        QDBusInterface existing(service, "/ClickMeow", service);
        existing.call("Show");
        return 0;
    }
#endif
    MeowApp meow;
#ifdef Q_OS_WIN
    QObject::connect(&server, &QLocalServer::newConnection, &meow, [&] {
        while (auto socket = server.nextPendingConnection()) {
            meow.Show();
            socket->disconnectFromServer();
            socket->deleteLater();
        }
    });
#endif
    return app.exec();
}
#include "main.moc"
