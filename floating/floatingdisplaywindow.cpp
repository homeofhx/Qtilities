#include "floatingdisplaywindow.h"

#include "floatinglabel.h"
#include "../countdown/countdowndisplay.h"
#include "../countdown/countdowntimer.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QScreen>
#include <QTimer>
#include <QWindow>

#ifdef Q_OS_MAC
#include "../RTC/macos_window.h"
#endif

namespace {
constexpr int TopOffset = 75;
}

FloatingDisplayWindow::FloatingDisplayWindow(QScreen *screen, CountdownTimer *countdownTimer)
    : targetScreen(screen) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool |
                   Qt::WindowTransparentForInput | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QWidget *container = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    localLabel = FloatingUi::createLabel(container, QStringLiteral("#CCFF9800"));
    utcLabel = FloatingUi::createLabel(container, QStringLiteral("#CC4CAF50"));
    countdownDisplay = new CountdownDisplay(countdownTimer, container);
    layout->addWidget(localLabel);
    layout->addWidget(utcLabel);
    layout->addWidget(countdownDisplay);
    setCentralWidget(container);

    winId();
    if (windowHandle()) windowHandle()->setScreen(screen);
    connect(screen, &QScreen::availableGeometryChanged, this, &FloatingDisplayWindow::recenter);
}

QScreen *FloatingDisplayWindow::screen() const {
    return targetScreen.data();
}

void FloatingDisplayWindow::setClockText(const QString &localHeadline, const QString &localDate,
                                         const QString &utcHeadline, const QString &utcDate) {
    localLabel->setText(FloatingUi::formatText(localHeadline, localDate));
    utcLabel->setText(FloatingUi::formatText(utcHeadline, utcDate));
}

void FloatingDisplayWindow::setContentVisibility(bool clockVisible, bool utcVisible,
                                                 bool countdownVisible) {
    localLabel->setVisible(clockVisible);
    utcLabel->setVisible(clockVisible && utcVisible);
    countdownDisplay->setVisible(countdownVisible);

    if (!clockVisible && !countdownVisible) {
        hide();
        return;
    }

    showAndCenter();
}

void FloatingDisplayWindow::showAndCenter() {
    show();
    adjustSize();
    recenter();

    // Native platforms can finalize the layout after the first show event.
    QTimer::singleShot(0, this, [this]() {
        adjustSize();
        recenter();
#ifdef Q_OS_MAC
        setMacOSFloatOnTop(windowHandle());
#endif
    });
}

void FloatingDisplayWindow::recenter() {
    if (!targetScreen) return;

    if (windowHandle()) windowHandle()->setScreen(targetScreen);
    const QRect geometry = targetScreen->availableGeometry();
    const int x = geometry.x() + (geometry.width() - width()) / 2;
    move(x, geometry.y() + TopOffset);
}
