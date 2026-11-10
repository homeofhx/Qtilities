#ifndef FLOATINGDISPLAYWINDOW_H
#define FLOATINGDISPLAYWINDOW_H

#include <QMainWindow>
#include <QPointer>

class CountdownDisplay;
class CountdownTimer;
class QLabel;
class QScreen;

class FloatingDisplayWindow : public QMainWindow {
public:
    FloatingDisplayWindow(QScreen *screen, CountdownTimer *countdownTimer);

    QScreen *screen() const;
    void setClockText(const QString &localHeadline, const QString &localDate,
                      const QString &utcHeadline, const QString &utcDate);
    void setContentVisibility(bool clockVisible, bool utcVisible, bool countdownVisible);

private:
    void recenter();
    void showAndCenter();

    QPointer<QScreen> targetScreen;
    QLabel *localLabel = nullptr;
    QLabel *utcLabel = nullptr;
    CountdownDisplay *countdownDisplay = nullptr;
};

#endif
