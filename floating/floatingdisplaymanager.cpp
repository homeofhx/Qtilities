#include "floatingdisplaymanager.h"

#include "floatingdisplaywindow.h"

#include <QDateTime>
#include <QGuiApplication>
#include <QScreen>
#include <algorithm>

FloatingDisplayManager::FloatingDisplayManager(QObject *parent) : QObject(parent) {
    connect(&countdownTimer, &CountdownTimer::displayVisibilityChanged,
            this, &FloatingDisplayManager::updateWindowVisibility);

    for (QScreen *screen : QGuiApplication::screens()) addScreen(screen);
    connect(qGuiApp, &QGuiApplication::screenAdded, this, &FloatingDisplayManager::addScreen);
    connect(qGuiApp, &QGuiApplication::screenRemoved, this, &FloatingDisplayManager::removeScreen);

    clockUpdateTimer.setInterval(1000);
    connect(&clockUpdateTimer, &QTimer::timeout, this, &FloatingDisplayManager::updateTime);
    clockUpdateTimer.start();
    updateTime();
}

FloatingDisplayManager::~FloatingDisplayManager() = default;

void FloatingDisplayManager::addScreen(QScreen *screen) {
    if (!screen) return;
    const auto alreadyPresent = std::any_of(displays.cbegin(), displays.cend(), [screen](const auto &display) {
        return display->screen() == screen;
    });
    if (alreadyPresent) return;

    displays.push_back(std::make_unique<FloatingDisplayWindow>(screen, &countdownTimer));
    updateTime();
    updateWindowVisibility();
}

void FloatingDisplayManager::removeScreen(QScreen *screen) {
    displays.erase(std::remove_if(displays.begin(), displays.end(), [screen](const auto &display) {
                       return display->screen() == screen;
                   }),
                   displays.end());
}

void FloatingDisplayManager::updateTime() {
    const QDateTime local = QDateTime::currentDateTime();
    const QDateTime utc = local.toUTC();
    const QString localHeadline = QStringLiteral("NOW %1").arg(local.toString(QStringLiteral("hh:mm:ss")));
    const QString localDate = local.toString(QStringLiteral("d MMM yyyy")).toUpper();
    const QString utcHeadline = QStringLiteral("UTC %1").arg(utc.toString(QStringLiteral("hh:mm:ss")));
    const QString utcDate = utc.toString(QStringLiteral("d MMM yyyy")).toUpper();

    for (const auto &display : displays) {
        display->setClockText(localHeadline, localDate, utcHeadline, utcDate);
    }
}

void FloatingDisplayManager::toggleUTC() {
    utcVisible = !utcVisible;
    updateWindowVisibility();
    emit utcVisibilityChanged(utcVisible);
}

void FloatingDisplayManager::showClock() {
    clockVisible = true;
    updateWindowVisibility();
}

void FloatingDisplayManager::hideClock() {
    clockVisible = false;
    updateWindowVisibility();
}

void FloatingDisplayManager::updateWindowVisibility() {
    const bool countdownVisible = countdownTimer.isDisplayed();
    for (const auto &display : displays) {
        display->setContentVisibility(clockVisible, utcVisible, countdownVisible);
    }
}
