#include "app.h"
#include "countdown/countdownsetupdialog.h"
#include <QIcon>
#include <QCursor>
#include <QCoreApplication>

ToolkitApp::ToolkitApp(QObject *parent) : QObject(parent) {
    awakeController = new AwakeController(this);
    clockWindow = new ClockWindow();
    countdownTimer = clockWindow->countdown();
    connect(clockWindow, &ClockWindow::utcVisibilityChanged, this, &ToolkitApp::onClockUTCVisibilityChanged);
    connect(countdownTimer, &CountdownTimer::stateChanged, this, &ToolkitApp::onCountdownStateChanged);
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

    // Countdown timer

    QAction *countdownHeader = new QAction("Timer", trayMenu);
    countdownHeader->setEnabled(false);
    trayMenu->addAction(countdownHeader);

    countdownSetupAction = new QAction("Set up", trayMenu);
    connect(countdownSetupAction, &QAction::triggered, this, &ToolkitApp::onCountdownSetup);
    trayMenu->addAction(countdownSetupAction);

    countdownPauseAction = new QAction("Pause", trayMenu);
    connect(countdownPauseAction, &QAction::triggered, this, &ToolkitApp::onCountdownPauseOrContinue);
    countdownPauseAction->setVisible(false);
    trayMenu->addAction(countdownPauseAction);

    countdownCancelAction = new QAction("Cancel", trayMenu);
    connect(countdownCancelAction, &QAction::triggered, this, &ToolkitApp::onCountdownCancel);
    countdownCancelAction->setVisible(false);
    trayMenu->addAction(countdownCancelAction);

    countdownDoneAction = new QAction("Done", trayMenu);
    connect(countdownDoneAction, &QAction::triggered, this, &ToolkitApp::onCountdownDone);
    countdownDoneAction->setVisible(false);
    trayMenu->addAction(countdownDoneAction);

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
    if (clockWindow->isClockVisible()) {
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

// Countdown

void ToolkitApp::onCountdownSetup() {
    CountdownSetupDialog dialog;
    if (dialog.exec() == QDialog::Accepted) {
        countdownTimer->startCountdown(dialog.durationSeconds(), dialog.countdownLabel());
    }
}

void ToolkitApp::onCountdownPauseOrContinue() {
    if (countdownTimer->state() == CountdownTimer::State::Running) {
        countdownTimer->pauseCountdown();
    } else if (countdownTimer->state() == CountdownTimer::State::Paused) {
        countdownTimer->continueCountdown();
    }
}

void ToolkitApp::onCountdownCancel() {
    countdownTimer->cancelCountdown();
}

void ToolkitApp::onCountdownDone() {
    countdownTimer->dismissFinishedCountdown();
}

void ToolkitApp::onCountdownStateChanged(CountdownTimer::State state) {
    const bool isActive = state == CountdownTimer::State::Running || state == CountdownTimer::State::Paused;
    countdownSetupAction->setVisible(state == CountdownTimer::State::Idle);
    countdownPauseAction->setVisible(isActive);
    countdownPauseAction->setText(state == CountdownTimer::State::Paused ? "Continue" : "Pause");
    countdownCancelAction->setVisible(isActive);
    countdownDoneAction->setVisible(state == CountdownTimer::State::Finished);
}

void ToolkitApp::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::Trigger) trayMenu->popup(QCursor::pos());
}

ToolkitApp::~ToolkitApp() {
    delete clockWindow;
}