#ifndef APP_H
#define APP_H
#include <QObject>
#include <QSystemTrayIcon>
#include "countdown/countdowntimer.h"

class QAction;
class AwakeController;
class Chime;
class FloatingDisplayManager;
class QMenu;

class ToolkitApp : public QObject {
    Q_OBJECT

public:
    explicit ToolkitApp(QObject *parent = nullptr);
    ~ToolkitApp() override;

private slots:
    void onAwakeToggled();
    void onShowTimeToggled();
    void onUTCToggled();
    void onChimeToggled();
    void onClockUTCVisibilityChanged(bool visible);
    void onCountdownSetup();
    void onCountdownPauseOrContinue();
    void onCountdownCancel();
    void onCountdownDone();
    void onCountdownStateChanged(CountdownTimer::State state);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void setupTrayIcon();

    AwakeController *awakeController = nullptr;
    FloatingDisplayManager *displayManager = nullptr;
    Chime *chime = nullptr;
    CountdownTimer *countdownTimer = nullptr;

    QSystemTrayIcon *trayIcon = nullptr;
    QMenu *trayMenu = nullptr;
    QAction *awakeToggleAction = nullptr;
    QAction *showTimeAction = nullptr;
    QAction *toggleUTCAction = nullptr;
    QAction *chimeToggleAction = nullptr;
    QAction *countdownSetupAction = nullptr;
    QAction *countdownPauseAction = nullptr;
    QAction *countdownCancelAction = nullptr;
    QAction *countdownDoneAction = nullptr;
};

#endif