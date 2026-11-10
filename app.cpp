#include "app.h"
#include "awake/awake.h"
#include "RTC/chime.h"
#include "countdown/countdownsetupdialog.h"
#include "floating/floatingdisplaymanager.h"
#include <QAction>
#include <QCoreApplication>
#include <QCursor>
#include <QIcon>
#include <QMenu>

ToolkitApp::ToolkitApp(QObject *parent) : QObject(parent) {
    awakeController = new AwakeController(this);
    displayManager = new FloatingDisplayManager(this);
    chime = new Chime(this);
    countdownTimer = displayManager->countdown();
    connect(displayManager, &FloatingDisplayManager::utcVisibilityChanged,
            this, &ToolkitApp::onClockUTCVisibilityChanged);
    connect(countdownTimer, &CountdownTimer::stateChanged, this, &ToolkitApp::onCountdownStateChanged);
    setupTrayIcon();
}

void ToolkitApp::setupTrayIcon() {
    trayMenu = new QMenu();

    // Awake

    QAction *awakeHeader = trayMenu->addAction("Awake");
    awakeHeader->setEnabled(false);

    awakeToggleAction = trayMenu->addAction("Awake!");
    connect(awakeToggleAction, &QAction::triggered, this, &ToolkitApp::onAwakeToggled);

    trayMenu->addSeparator();

    // RTC

    QAction *rtcHeader = trayMenu->addAction("RTC");
    rtcHeader->setEnabled(false);

    showTimeAction = trayMenu->addAction("Show Time");
    connect(showTimeAction, &QAction::triggered, this, &ToolkitApp::onShowTimeToggled);

    toggleUTCAction = trayMenu->addAction(displayManager->isUTCVisible() ? "Hide UTC" : "Show UTC");
    connect(toggleUTCAction, &QAction::triggered, this, &ToolkitApp::onUTCToggled);

    chimeToggleAction = trayMenu->addAction("Chime On");
    connect(chimeToggleAction, &QAction::triggered, this, &ToolkitApp::onChimeToggled);

    trayMenu->addSeparator();

    // Countdown timer

    QAction *countdownHeader = trayMenu->addAction("Timer");
    countdownHeader->setEnabled(false);

    countdownSetupAction = trayMenu->addAction("Set up");
    connect(countdownSetupAction, &QAction::triggered, this, &ToolkitApp::onCountdownSetup);

    countdownPauseAction = trayMenu->addAction("Pause");
    connect(countdownPauseAction, &QAction::triggered, this, &ToolkitApp::onCountdownPauseOrContinue);
    countdownPauseAction->setVisible(false);

    countdownCancelAction = trayMenu->addAction("Cancel");
    connect(countdownCancelAction, &QAction::triggered, this, &ToolkitApp::onCountdownCancel);
    countdownCancelAction->setVisible(false);

    countdownDoneAction = trayMenu->addAction("Done");
    connect(countdownDoneAction, &QAction::triggered, this, &ToolkitApp::onCountdownDone);
    countdownDoneAction->setVisible(false);

    trayMenu->addSeparator();

    // Quit

    QAction *quitAction = trayMenu->addAction("Quit!");
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

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
    if (displayManager->isClockVisible()) {
        displayManager->hideClock();
        showTimeAction->setText("Show Time");
    } else {
        displayManager->showClock();
        showTimeAction->setText("Hide Time");
    }
}

void ToolkitApp::onUTCToggled() {
    displayManager->toggleUTC();
}

void ToolkitApp::onChimeToggled() {
    const bool newState = !chime->isEnabled();
    chime->setEnabled(newState);
    chimeToggleAction->setText(newState ? "Chime Off" : "Chime On");
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
    trayIcon->hide();
    trayIcon->setContextMenu(nullptr);
    delete trayMenu;
}