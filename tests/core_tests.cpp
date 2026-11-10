#include "../RTC/chimerenderer.h"
#include "../countdown/countdowntimer.h"

#include <QAudioFormat>
#include <QSignalSpy>
#include <QtTest>

class CoreTests : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void countdownInitialState();
    void countdownLifecycle();
    void countdownFinishes();
    void chimeRendererProducesExpectedPcm();
};

void CoreTests::initTestCase() {
    qRegisterMetaType<CountdownTimer::State>();
}

void CoreTests::countdownInitialState() {
    CountdownTimer timer;

    QCOMPARE(timer.state(), CountdownTimer::State::Idle);
    QCOMPARE(timer.isDisplayed(), false);
    QCOMPARE(timer.timeText(), QStringLiteral("-00:00:00"));
    QCOMPARE(timer.labelText(), QString());
    QCOMPARE(timer.isDisplayBright(), true);

    timer.startCountdown(0, QStringLiteral("Ignored"));
    QCOMPARE(timer.state(), CountdownTimer::State::Idle);
}

void CoreTests::countdownLifecycle() {
    CountdownTimer timer;
    QSignalSpy stateSpy(&timer, &CountdownTimer::stateChanged);
    QSignalSpy visibilitySpy(&timer, &CountdownTimer::displayVisibilityChanged);

    timer.startCountdown(90, QStringLiteral("Deploy <prod>"));
    QCOMPARE(timer.state(), CountdownTimer::State::Running);
    QCOMPARE(timer.timeText(), QStringLiteral("-00:01:30"));
    QCOMPARE(timer.labelText(), QStringLiteral("Deploy <prod>"));
    QCOMPARE(visibilitySpy.count(), 1);
    QCOMPARE(visibilitySpy.takeFirst().at(0).toBool(), true);

    timer.pauseCountdown();
    QCOMPARE(timer.state(), CountdownTimer::State::Paused);

    timer.continueCountdown();
    QCOMPARE(timer.state(), CountdownTimer::State::Running);

    timer.cancelCountdown();
    QCOMPARE(timer.state(), CountdownTimer::State::Idle);
    QCOMPARE(timer.isDisplayed(), false);
    QCOMPARE(timer.labelText(), QString());
    QCOMPARE(visibilitySpy.count(), 1);
    QCOMPARE(visibilitySpy.takeFirst().at(0).toBool(), false);
    QCOMPARE(stateSpy.count(), 4);
}

void CoreTests::countdownFinishes() {
    CountdownTimer timer;
    timer.startCountdown(1, QStringLiteral("Short timer"));

    QTRY_COMPARE_WITH_TIMEOUT(timer.state(), CountdownTimer::State::Finished, 1500);
    QCOMPARE(timer.timeText(), QStringLiteral("-00:00:00"));
    QCOMPARE(timer.isDisplayed(), true);

    timer.dismissFinishedCountdown();
    QCOMPARE(timer.state(), CountdownTimer::State::Idle);
}

void CoreTests::chimeRendererProducesExpectedPcm() {
    QAudioFormat format;
    format.setSampleRate(8000);
    format.setChannelCount(1);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    format.setSampleFormat(QAudioFormat::Int16);
#else
    format.setSampleSize(16);
    format.setCodec(QStringLiteral("audio/pcm"));
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);
#endif

    QVERIFY(ChimeRenderer::canRender(format));
    QCOMPARE(ChimeRenderer::hourPreludeMilliseconds(), 21000);
    QCOMPARE(ChimeRenderer::renderQuarterChime(7, format), QByteArray());
    QCOMPARE(ChimeRenderer::renderQuarterChime(15, format).size(), 83200);
    QCOMPARE(ChimeRenderer::renderHourStrike(format).size(), 72000);
}

QTEST_GUILESS_MAIN(CoreTests)

#include "core_tests.moc"
