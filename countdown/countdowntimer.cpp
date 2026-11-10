#include "countdowntimer.h"
#include <QDateTime>

namespace {
constexpr int kUpdateIntervalMs = 100;
constexpr int kFlashIntervalMs = 500;
}

CountdownTimer::CountdownTimer(QObject *parent) : QObject(parent) {
    updateTimer.setInterval(kUpdateIntervalMs);
    connect(&updateTimer, &QTimer::timeout, this, &CountdownTimer::updateRemainingTime);

    flashTimer.setInterval(kFlashIntervalMs);
    connect(&flashTimer, &QTimer::timeout, this, &CountdownTimer::toggleFlash);

    updateDisplay();
}

void CountdownTimer::startCountdown(qint64 durationSeconds, const QString &label) {
    if (durationSeconds <= 0) return;

    countdownLabel = label;
    remainingMs = durationSeconds * 1000;
    deadlineMs = QDateTime::currentMSecsSinceEpoch() + remainingMs;
    flashTimer.stop();
    setDisplayBrightness(true);
    updateDisplay();
    setState(State::Running);
    updateTimer.start();
    emit displayVisibilityChanged(true);
}

void CountdownTimer::pauseCountdown() {
    if (currentState != State::Running) return;

    remainingMs = qMax<qint64>(0, deadlineMs - QDateTime::currentMSecsSinceEpoch());
    if (remainingMs == 0) {
        finishCountdown();
        return;
    }

    updateTimer.stop();
    updateDisplay();
    setState(State::Paused);
}

void CountdownTimer::continueCountdown() {
    if (currentState != State::Paused) return;

    deadlineMs = QDateTime::currentMSecsSinceEpoch() + remainingMs;
    setState(State::Running);
    updateTimer.start();
}

void CountdownTimer::cancelCountdown() {
    if (currentState == State::Idle) return;

    updateTimer.stop();
    flashTimer.stop();
    remainingMs = 0;
    countdownLabel.clear();
    setDisplayBrightness(true);
    updateDisplay();
    setState(State::Idle);
    emit displayVisibilityChanged(false);
}

void CountdownTimer::dismissFinishedCountdown() {
    if (currentState == State::Finished) cancelCountdown();
}

void CountdownTimer::updateRemainingTime() {
    if (currentState != State::Running) return;

    remainingMs = qMax<qint64>(0, deadlineMs - QDateTime::currentMSecsSinceEpoch());
    updateDisplay();
    if (remainingMs == 0) finishCountdown();
}

void CountdownTimer::toggleFlash() {
    setDisplayBrightness(!displayIsBright);
}

void CountdownTimer::finishCountdown() {
    updateTimer.stop();
    remainingMs = 0;
    updateDisplay();
    setState(State::Finished);
    setDisplayBrightness(true);
    flashTimer.start();
}

void CountdownTimer::setState(State state) {
    if (currentState == state) return;
    currentState = state;
    emit stateChanged(currentState);
}

void CountdownTimer::updateDisplay() {
    const qint64 totalSeconds = (remainingMs + 999) / 1000;
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;

    const QString newTime = QStringLiteral("-%1:%2:%3")
                                .arg(hours, 2, 10, QLatin1Char('0'))
                                .arg(minutes, 2, 10, QLatin1Char('0'))
                                .arg(seconds, 2, 10, QLatin1Char('0'));
    if (newTime == formattedTime && countdownLabel == displayedLabel) return;

    formattedTime = newTime;
    displayedLabel = countdownLabel;
    emit displayChanged(formattedTime, countdownLabel);
}

void CountdownTimer::setDisplayBrightness(bool bright) {
    if (displayIsBright == bright) return;
    displayIsBright = bright;
    emit displayBrightnessChanged(displayIsBright);
}