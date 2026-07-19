#include "app.h"
#include <QIcon>
#include <QCursor>
#include <QCoreApplication>

ToolkitApp::ToolkitApp(QObject *parent) : QObject(parent) {
    awakeController = new AwakeController(this);
    clockWindow = new ClockWindow();
    connect(clockWindow, &ClockWindow::utcVisibilityChanged, this, &ToolkitApp::onClockUTCVisibilityChanged);
    setupTrayIcon();
}

void ToolkitApp::setupTrayIcon() {
    trayMenu = new QMenu();

    // Awake

    QAction *awakeHeader = new QAction("Awake", trayMenu);
    awakeHeader->setEnabled(false);
    trayMenu->addAction(awakeHeader);

    awakeToggleAction = new QAction("Awake!", trayMenu);
    connect(awakeToggleAction, &QAction::triggered, this, &ToolkitApp::onAwakeToggled);
    trayMenu->addAction(awakeToggleAction);

    trayMenu->addSeparator();

    // RTC

    QAction *rtcHeader = new QAction("RTC", trayMenu);
    rtcHeader->setEnabled(false);
    trayMenu->addAction(rtcHeader);

    showTimeAction = new QAction("Show Time", trayMenu);
    connect(showTimeAction, &QAction::triggered, this, &ToolkitApp::onShowTimeToggled);
    trayMenu->addAction(showTimeAction);

    toggleUTCAction = new QAction(clockWindow->isUTCVisible() ? "Hide UTC" : "Show UTC", trayMenu);
    connect(toggleUTCAction, &QAction::triggered, this, &ToolkitApp::onUTCToggled);
    trayMenu->addAction(toggleUTCAction);

    trayMenu->addSeparator();

    // Quit
    QAction *quitAction = new QAction("Quit!", trayMenu);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    trayMenu->addAction(quitAction);

    // Taskbar icon config
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->setIcon(QIcon(":/AppIcon.png"));
    connect(trayIcon, &QSystemTrayIcon::activated, this, &ToolkitApp::onTrayIconActivated);
    trayIcon->show();
}

// Awake

void ToolkitApp::onAwakeToggled() {
    const bool newState = !awakeController->isActive();
    awakeController->setActive(newState);
    awakeToggleAction->setText(newState ? "Deactivate Awake" : "Awake!");
}

// RTC

void ToolkitApp::onShowTimeToggled() {
    if (clockWindow->isVisible()) {
        clockWindow->hideClock();
        showTimeAction->setText("Show Time");
    } else {
        clockWindow->showClock();
        showTimeAction->setText("Hide Time");
    }
}

void ToolkitApp::onUTCToggled() {
    clockWindow->toggleUTC();
}

void ToolkitApp::onClockUTCVisibilityChanged(bool visible) {
    toggleUTCAction->setText(visible ? "Hide UTC" : "Show UTC");
}

void ToolkitApp::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) trayMenu->popup(QCursor::pos());
}

ToolkitApp::~ToolkitApp() {
    delete clockWindow;
}