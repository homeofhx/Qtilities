#ifndef CLOCKWINDOW_H
#define CLOCKWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>

class CountdownTimer;

class ClockWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ClockWindow(QWidget *parent = nullptr);

    bool isUTCVisible() const { return showUTC; }
    bool isClockVisible() const { return clockVisible; }
    CountdownTimer *countdown() const { return countdownTimer; }

public slots:
    void toggleUTC();
    void showClock();
    void hideClock();

signals:
    void utcVisibilityChanged(bool visible);

private slots:
    void updateTime();
    void updateWindowVisibility();

private:
    void moveToTopCenter();

    bool showUTC = true;
    bool clockVisible = false;

    QLabel *localLabel;
    QLabel *utcLabel;
    CountdownTimer *countdownTimer;
    QTimer *timer;
};

#endif