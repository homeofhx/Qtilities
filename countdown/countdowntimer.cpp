#include "countdowntimer.h"

#include <QDateTime>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>

namespace {
constexpr int kUpdateIntervalMs = 100;
constexpr int kFlashIntervalMs = 500;
}

CountdownTimer::CountdownTimer(QWidget *parent) : QWidget(parent) {
    setFixedWidth(500);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    displayLabel = new QLabel(this);
    displayLabel->setAlignment(Qt::AlignCenter);
    displayLabel->setTextFormat(Qt::RichText);
    applyDisplayStyle(true);

    QGraphicsDropShadowEffect *outline = new QGraphicsDropShadowEffect(this);
    outline->setBlurRadius(30);
    outline->setColor(Qt::white);
    outline->setOffset(0);
    displayLabel->setGraphicsEffect(outline);
    layout->addWidget(displayLabel);

    updateTimer = new QTimer(this);
    updateTimer->setInterval(kUpdateIntervalMs);
    connect(updateTimer, &QTimer::timeout, this, &CountdownTimer::updateRemainingTime);

    flashTimer = new QTimer(this);
    flashTimer->setInterval(kFlashIntervalMs);
    connect(flashTimer, &QTimer::timeout, this, &CountdownTimer::toggleFlash);

    updateDisplay();
}

void CountdownTimer::startCountdown(qint64 durationSeconds, const QString &label) {
    if (durationSeconds <= 0) return;

    labelText = label;
    remainingMs = durationSeconds * 1000;
    deadlineMs = QDateTime::currentMSecsSinceEpoch() + remainingMs;
    flashTimer->stop();
    flashIsBright = true;
    applyDisplayStyle(true);
    updateDisplay();
    setState(State::Running);
    updateTimer->start();
    emit displayVisibilityChanged(true);
}

void CountdownTimer::pauseCountdown() {
    if (currentState != State::Running) return;

    remainingMs = qMax<qint64>(0, deadlineMs - QDateTime::currentMSecsSinceEpoch());
    if (remainingMs == 0) {
        finishCountdown();
        return;
    }

    updateTimer->stop();
    updateDisplay();
    setState(State::Paused);
}

void CountdownTimer::continueCountdown() {
    if (currentState != State::Paused) return;

    deadlineMs = QDateTime::currentMSecsSinceEpoch() + remainingMs;
    setState(State::Running);
    updateTimer->start();
}

void CountdownTimer::cancelCountdown() {
    if (currentState == State::Idle) return;

    updateTimer->stop();
    flashTimer->stop();
    remainingMs = 0;
    labelText.clear();
    flashIsBright = true;
    applyDisplayStyle(true);
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
    flashIsBright = !flashIsBright;
    applyDisplayStyle(flashIsBright);
}

void CountdownTimer::finishCountdown() {
    updateTimer->stop();
    remainingMs = 0;
    updateDisplay();
    setState(State::Finished);
    flashIsBright = true;
    applyDisplayStyle(true);
    flashTimer->start();
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

    const QString timeText = QString("-%1:%2:%3")
                                 .arg(hours, 2, 10, QLatin1Char('0'))
                                 .arg(minutes, 2, 10, QLatin1Char('0'))
                                 .arg(seconds, 2, 10, QLatin1Char('0'));
    displayLabel->setText(QString("%1<br><span style='font-size: 32px; font-weight: normal;'>%2</span>")
                              .arg(timeText, labelText.toHtmlEscaped()));
}

void CountdownTimer::applyDisplayStyle(bool bright) {
    const QString color = bright ? "#CCF44336" : "#22F44336";
    displayLabel->setStyleSheet(QString("color: %1;"
                                        "font-family: 'Consolas', 'Menlo', monospace;"
                                        "font-size: 50px;"
                                        "font-weight: bold;"
                                        "background: transparent;")
                                    .arg(color));
}