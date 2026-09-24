#include <effect/effecthandler.h>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QQmlExtensionPlugin>
#include <qqml.h>

// Observe presses without grabbing, consuming, or modifying input events.
class ClickBridge : public QObject {
    Q_OBJECT
public:
    explicit ClickBridge(QObject *parent = nullptr) : QObject(parent) {
        connect(KWin::effects, &KWin::EffectsHandler::mouseChanged, this,
                [](const QPointF &, const QPointF &, Qt::MouseButtons now,
                   Qt::MouseButtons before, Qt::KeyboardModifiers, Qt::KeyboardModifiers) {
            if ((now & Qt::LeftButton) && !(before & Qt::LeftButton)
                && !KWin::effects->isScreenLocked()) {
                auto message = QDBusMessage::createSignal(
                    "/ClickMeow", "local.clickmeow.Input", "LeftPressed");
                QDBusConnection::sessionBus().send(message);
            }
        });
    }
};

class MeowPlugin : public QQmlExtensionPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override {
        qmlRegisterType<ClickBridge>(uri, 1, 0, "ClickBridge");
    }
};
#include "bridge.moc"
