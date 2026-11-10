#ifndef COUNTDOWNDISPLAY_H
#define COUNTDOWNDISPLAY_H
#include <QWidget>

class CountdownTimer;
class QLabel;

class CountdownDisplay : public QWidget {
public:
    explicit CountdownDisplay(CountdownTimer *timer, QWidget *parent = nullptr);

private:
    void updateText(const QString &timeText, const QString &labelText);
    void updateBrightness(bool bright);

    QLabel *displayLabel = nullptr;
};

#endif