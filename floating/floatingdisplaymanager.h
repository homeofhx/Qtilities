#ifndef FLOATINGDISPLAYMANAGER_H
#define FLOATINGDISPLAYMANAGER_H

#include "../countdown/countdowntimer.h"

#include <QObject>
#include <QTimer>
#include <memory>
#include <vector>

class FloatingDisplayWindow;
class QScreen;

class FloatingDisplayManager : public QObject {
    Q_OBJECT

public:
    explicit FloatingDisplayManager(QObject *parent = nullptr);
    ~FloatingDisplayManager() override;

    bool isUTCVisible() const { return utcVisible; }
    bool isClockVisible() const { return clockVisible; }
    CountdownTimer *countdown() { return &countdownTimer; }

public slots:
    void toggleUTC();
    void showClock();
    void hideClock();

signals:
    void utcVisibilityChanged(bool visible);

private:
    void addScreen(QScreen *screen);
    void removeScreen(QScreen *screen);
    void updateTime();
    void updateWindowVisibility();

    bool utcVisible = true;
    bool clockVisible = false;
    CountdownTimer countdownTimer;
    QTimer clockUpdateTimer;
    std::vector<std::unique_ptr<FloatingDisplayWindow>> displays;
};

#endif
