#include "clockwindow.h"
#include "../countdown/countdowntimer.h"
#include <QDateTime>
#include <QScreen>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

#ifdef Q_OS_MAC
#include "macos_window.h"
#endif

ClockWindow::ClockWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool | Qt::WindowTransparentForInput | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QWidget *container = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(20);

    auto setupLabel = [this](const QString &color) {
        QLabel *label = new QLabel(this);
        label->setFixedWidth(500);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet(QString("color: %1;"
                                     "font-family: 'Consolas', 'Menlo', monospace;"
                                     "font-size: 50px;"
                                     "font-weight: bold;"
                                     "background: transparent;").arg(color));

        QGraphicsDropShadowEffect *outline = new QGraphicsDropShadowEffect(this);
        outline->setBlurRadius(30);
        outline->setColor(Qt::white);
        outline->setOffset(0);
        label->setGraphicsEffect(outline);

        return label;
    };

    localLabel = setupLabel("#CCFF9800");
    utcLabel = setupLabel("#CC4CAF50");
    countdownTimer = new CountdownTimer(this);

    layout->addWidget(localLabel);
    layout->addWidget(utcLabel);
    layout->addWidget(countdownTimer);
    countdownTimer->hide();
    connect(countdownTimer, &CountdownTimer::displayVisibilityChanged, this, &ClockWindow::updateWindowVisibility);

    setCentralWidget(container);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ClockWindow::updateTime);
    timer->start(1000);
    updateTime();
    adjustSize();
    moveToTopCenter();
}

void ClockWindow::updateTime() {
    // Local
    QDateTime now = QDateTime::currentDateTime();
    QString localTime = now.toString("hh:mm:ss");
    QString localDate = now.toString("d MMM yyyy").toUpper();
    localLabel->setText(QString("NOW %1<br>" "<span style='font-size: %2px; font-weight: normal;'>%3</span>")
                            .arg(localTime)
                            .arg(qRound(65 * 0.5))
                            .arg(localDate));

    // UTC
    QDateTime utc = QDateTime::currentDateTimeUtc();
    QString utcTime = utc.toString("hh:mm:ss");
    QString utcDate = utc.toString("d MMM yyyy").toUpper();
    utcLabel->setText(QString("UTC %1<br>" "<span style='font-size: %2px; font-weight: normal;'>%3</span>")
                          .arg(utcTime)
                          .arg(qRound(65 * 0.5))
                          .arg(utcDate));
}

void ClockWindow::toggleUTC() {
    showUTC = !showUTC;
    utcLabel->setVisible(clockVisible && showUTC);
    adjustSize();
    moveToTopCenter();
    emit utcVisibilityChanged(showUTC);
}

void ClockWindow::showClock() {
    clockVisible = true;
    updateWindowVisibility();
}

void ClockWindow::hideClock() {
    clockVisible = false;
    updateWindowVisibility();
}

void ClockWindow::updateWindowVisibility() {
    localLabel->setVisible(clockVisible);
    utcLabel->setVisible(clockVisible && showUTC);
    countdownTimer->setVisible(countdownTimer->isDisplayed());

    adjustSize();
    moveToTopCenter();

    if (!clockVisible && !countdownTimer->isDisplayed()) {
        hide();
        return;
    }

    show();

    // Dealing with Mac OS's floating view issue
#ifdef Q_OS_MAC
    QTimer::singleShot(0, this, [this]() { setMacOSFloatOnTop(this->windowHandle()); });
#endif
}

void ClockWindow::moveToTopCenter() {
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    const QRect geometry = screen->availableGeometry();
    const int x = geometry.x() + (geometry.width() - width()) / 2;
    move(x, geometry.y() + 75);
}