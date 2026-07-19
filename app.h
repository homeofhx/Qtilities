#ifndef APP_H
#define APP_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "awake/awake.h"
#include "RTC/clockwindow.h"

class ToolkitApp : public QObject {
    Q_OBJECT

public:
    explicit ToolkitApp(QObject *parent = nullptr);
    ~ToolkitApp() override;

private slots:
    void onAwakeToggled();
    void onShowTimeToggled();
    void onUTCToggled();
    void onClockUTCVisibilityChanged(bool visible);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void setupTrayIcon();

    AwakeController *awakeController;
    ClockWindow *clockWindow;

    QSystemTrayIcon *trayIcon = nullptr;
    QMenu *trayMenu = nullptr;
    QAction *awakeToggleAction = nullptr;
    QAction *showTimeAction = nullptr;
    QAction *toggleUTCAction = nullptr;
};

#endif