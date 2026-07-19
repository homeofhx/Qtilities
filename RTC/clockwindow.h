#ifndef CLOCKWINDOW_H
#define CLOCKWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>

class ClockWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ClockWindow(QWidget *parent = nullptr);

    bool isUTCVisible() const { return showUTC; }

public slots:
    void toggleUTC();
    void showClock();
    void hideClock();

signals:
    void utcVisibilityChanged(bool visible);

private slots:
    void updateTime();

private:
    bool showUTC = true;

    QLabel *localLabel;
    QLabel *utcLabel;
    QTimer *timer;
};

#endif