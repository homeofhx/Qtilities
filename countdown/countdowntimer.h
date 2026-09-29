#ifndef COUNTDOWNTIMER_H
#define COUNTDOWNTIMER_H

#include <QWidget>

class QLabel;
class QTimer;

class CountdownTimer : public QWidget {
    Q_OBJECT

public:
    enum class State {
        Idle,
        Running,
        Paused,
        Finished
    };
    Q_ENUM(State)

    explicit CountdownTimer(QWidget *parent = nullptr);

    State state() const { return currentState; }
    bool isDisplayed() const { return currentState != State::Idle; }

public slots:
    void startCountdown(qint64 durationSeconds, const QString &label);
    void pauseCountdown();
    void continueCountdown();
    void cancelCountdown();
    void dismissFinishedCountdown();

signals:
    void stateChanged(CountdownTimer::State state);
    void displayVisibilityChanged(bool visible);

private slots:
    void updateRemainingTime();
    void toggleFlash();

private:
    void finishCountdown();
    void setState(State state);
    void updateDisplay();
    void applyDisplayStyle(bool bright);

    State currentState = State::Idle;
    QLabel *displayLabel = nullptr;
    QTimer *updateTimer = nullptr;
    QTimer *flashTimer = nullptr;
    QString labelText;
    qint64 deadlineMs = 0;
    qint64 remainingMs = 0;
    bool flashIsBright = true;
};

#endif