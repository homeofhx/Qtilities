#ifndef COUNTDOWNTIMER_H
#define COUNTDOWNTIMER_H
#include <QObject>
#include <QString>
#include <QTimer>

class CountdownTimer : public QObject {
    Q_OBJECT

public:
    enum class State {
        Idle,
        Running,
        Paused,
        Finished
    };
    Q_ENUM(State)

    explicit CountdownTimer(QObject *parent = nullptr);

    State state() const { return currentState; }
    bool isDisplayed() const { return currentState != State::Idle; }
    bool isDisplayBright() const { return displayIsBright; }
    const QString &timeText() const { return formattedTime; }
    const QString &labelText() const { return countdownLabel; }

public slots:
    void startCountdown(qint64 durationSeconds, const QString &label);
    void pauseCountdown();
    void continueCountdown();
    void cancelCountdown();
    void dismissFinishedCountdown();

signals:
    void stateChanged(CountdownTimer::State state);
    void displayVisibilityChanged(bool visible);
    void displayChanged(const QString &timeText, const QString &labelText);
    void displayBrightnessChanged(bool bright);

private slots:
    void updateRemainingTime();
    void toggleFlash();

private:
    void finishCountdown();
    void setState(State state);
    void updateDisplay();
    void setDisplayBrightness(bool bright);

    State currentState = State::Idle;
    QTimer updateTimer;
    QTimer flashTimer;
    QString formattedTime;
    QString countdownLabel;
    QString displayedLabel;
    qint64 deadlineMs = 0;
    qint64 remainingMs = 0;
    bool displayIsBright = true;
};

#endif