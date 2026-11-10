#include "countdowndisplay.h"
#include "countdowntimer.h"
#include "../floating/floatinglabel.h"
#include <QLabel>
#include <QVBoxLayout>

CountdownDisplay::CountdownDisplay(CountdownTimer *timer, QWidget *parent) : QWidget(parent) {
    Q_ASSERT(timer);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    displayLabel = FloatingUi::createLabel(this, QStringLiteral("#CCF44336"));
    layout->addWidget(displayLabel);

    connect(timer, &CountdownTimer::displayChanged, this, &CountdownDisplay::updateText);
    connect(timer, &CountdownTimer::displayBrightnessChanged, this, &CountdownDisplay::updateBrightness);
    updateText(timer->timeText(), timer->labelText());
    updateBrightness(timer->isDisplayBright());
}

void CountdownDisplay::updateText(const QString &timeText, const QString &labelText) {
    displayLabel->setText(FloatingUi::formatText(timeText, labelText));
}

void CountdownDisplay::updateBrightness(bool bright) {
    FloatingUi::setLabelColor(displayLabel, bright ? QStringLiteral("#CCF44336") : QStringLiteral("#22F44336"));
}