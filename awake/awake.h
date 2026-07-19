#ifndef AWAKE_H
#define AWAKE_H

#include <QObject>

#ifdef Q_OS_MAC
#include <IOKit/pwr_mgt/IOPMLib.h>
#endif

#ifdef Q_OS_LINUX
#include <QtDBus/QDBusInterface>
#endif

class AwakeController : public QObject {
    Q_OBJECT

public:
    explicit AwakeController(QObject *parent = nullptr);
    ~AwakeController() override;

    bool isActive() const { return active; }
    void setActive(bool on);

private:
    void activate();
    void deactivate();

    bool active = false;

#ifdef Q_OS_MAC
    IOPMAssertionID assertionID = 0;
#endif

#ifdef Q_OS_LINUX
    QDBusInterface *dbusInterface = nullptr;
    uint linuxCookie = 0;
#endif
};

#endif