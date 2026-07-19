#include "awake.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_LINUX
#include <QDBusConnection>
#include <QDBusReply>
#endif

AwakeController::AwakeController(QObject *parent) : QObject(parent) {
#ifdef Q_OS_LINUX
    dbusInterface = new QDBusInterface("org.freedesktop.ScreenSaver",
                                        "/org/freedesktop/ScreenSaver",
                                        "org.freedesktop.ScreenSaver",
                                        QDBusConnection::sessionBus(),
                                        this);
#endif
}

void AwakeController::setActive(bool on) {
    if (on == active) return;

    if (on) {
        activate();
    } else {
        deactivate();
    }

    active = on;
}

void AwakeController::activate() {
#ifdef Q_OS_WIN
    SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
#endif

#ifdef Q_OS_MAC
    CFStringRef reasonForActivity = CFSTR("Toolkit is preventing this computer from sleep");
    IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep,
                                kIOPMAssertionLevelOn,
                                reasonForActivity,
                                &assertionID);
#endif

#ifdef Q_OS_LINUX
    if (dbusInterface && dbusInterface->isValid()) {
        QDBusReply<uint> reply = dbusInterface->call("Inhibit", "Toolkit", "Awake Active");
        if (reply.isValid()) linuxCookie = reply.value();
    }
#endif
}

void AwakeController::deactivate() {
#ifdef Q_OS_WIN
    SetThreadExecutionState(ES_CONTINUOUS);
#endif

#ifdef Q_OS_MAC
    if (assertionID != 0) {
        IOPMAssertionRelease(assertionID);
        assertionID = 0;
    }
#endif

#ifdef Q_OS_LINUX
    if (linuxCookie > 0 && dbusInterface && dbusInterface->isValid()) {
        dbusInterface->call("UnInhibit", linuxCookie);
        linuxCookie = 0;
    }
#endif
}

AwakeController::~AwakeController() {
    if (active) deactivate();
}