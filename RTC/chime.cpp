#include "chime.h"
#include "chimerenderer.h"
#include <QSysInfo>
#include <QAudioDeviceInfo>
#include <QAudioOutput>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioDevice>
#include <QAudioSink>
#include <QMediaDevices>
#else
#endif

namespace {
constexpr int QuarterMilliseconds = 15 * 60 * 1000;
constexpr int HourMilliseconds = 60 * 60 * 1000;
constexpr int MaximumChimeLatenessMilliseconds = 5000;
constexpr int MaximumPreludeLatenessMilliseconds = 250;
constexpr int MaximumScheduleIntervalMilliseconds = 1000;

QDateTime recentQuarterBoundary(const QDateTime &now) {
    const QTime time = now.time();
    const int elapsed = time.second() * 1000 + time.msec();
    if (time.minute() == 0 || time.minute() % 15 != 0 || elapsed > MaximumChimeLatenessMilliseconds) return {};
    return now.addMSecs(-elapsed);
}

QDateTime recentHourBoundary(const QDateTime &now) {
    const QTime time = now.time();
    const int elapsed = time.second() * 1000 + time.msec();
    if (time.minute() != 0 || elapsed > MaximumChimeLatenessMilliseconds) return {};
    return now.addMSecs(-elapsed);
}

QDateTime recentHourPrelude(const QDateTime &now) {
    const QTime time = now.time();
    if (time.minute() != 59) return {};

    const int preludeDuration = ChimeRenderer::hourPreludeMilliseconds();
    const int preludeOffset = 60 * 1000 - preludeDuration;
    const int millisecondsIntoMinute = time.second() * 1000 + time.msec();
    const int elapsed = millisecondsIntoMinute - preludeOffset;
    if (elapsed < 0 || elapsed > MaximumPreludeLatenessMilliseconds) return {};

    const QDateTime prelude = now.addMSecs(-elapsed);
    const QTime targetTime = prelude.addMSecs(preludeDuration).time();

    if (targetTime.minute() != 0 || targetTime.second() != 0 || targetTime.msec() != 0) return {};
    return prelude;
}
}

Chime::Chime(QObject *parent) : QObject(parent) {
    schedulerTimer.setSingleShot(true);
    schedulerTimer.setTimerType(Qt::PreciseTimer);
    connect(&schedulerTimer, &QTimer::timeout, this, &Chime::onSchedulerTimeout);
}

Chime::~Chime() {
    resetAudioOutput();
}

void Chime::setEnabled(bool shouldEnable) {
    if (enabled == shouldEnable) return;

    enabled = shouldEnable;
    if (enabled) {
        const QDateTime now = QDateTime::currentDateTime();
        const QDateTime hourBoundary = recentHourBoundary(now);
        const QDateTime hourPrelude = recentHourPrelude(now);
        lastPlayedStrikeBoundary = hourBoundary;
        if (hourBoundary.isValid()) {
            lastPlayedMelodyBoundary = hourBoundary;
        } else if (hourPrelude.isValid()) {
            lastPlayedMelodyBoundary = hourPrelude.addMSecs(ChimeRenderer::hourPreludeMilliseconds());
        } else {
            lastPlayedMelodyBoundary = recentQuarterBoundary(now);
        }
        scheduleNextChimeEvent();
        if (prepareAudioOutput()) cacheChimes();
    } else {
        schedulerTimer.stop();
        lastPlayedMelodyBoundary = QDateTime();
        lastPlayedStrikeBoundary = QDateTime();
        resetAudioOutput();
    }
}

void Chime::scheduleNextChimeEvent() {
    if (!enabled) return;

    const QDateTime now = QDateTime::currentDateTime();
    const QTime time = now.time();
    const int millisecondsIntoQuarter = ((time.minute() % 15) * 60 + time.second()) * 1000 + time.msec();
    const int quarterDelay = QuarterMilliseconds - millisecondsIntoQuarter;
    const int millisecondsIntoHour = (time.minute() * 60 + time.second()) * 1000 + time.msec();
    const int preludeDuration = ChimeRenderer::hourPreludeMilliseconds();
    const int preludeDelay = millisecondsIntoHour < HourMilliseconds - preludeDuration
                                 ? HourMilliseconds - preludeDuration - millisecondsIntoHour
                                 : 2 * HourMilliseconds - preludeDuration - millisecondsIntoHour;
    const int delay = qMin(quarterDelay, preludeDelay);
    schedulerTimer.start(qMin(delay, MaximumScheduleIntervalMilliseconds));
}

void Chime::onSchedulerTimeout() {
    if (!enabled) return;

    const QDateTime now = QDateTime::currentDateTime();
    const QDateTime hourBoundary = recentHourBoundary(now);
    if (hourBoundary.isValid()
        && (!lastPlayedStrikeBoundary.isValid() || hourBoundary > lastPlayedStrikeBoundary)
        && playHourStrikes(hourBoundary)) {
        lastPlayedStrikeBoundary = hourBoundary;
    }

    const QDateTime prelude = recentHourPrelude(now);
    if (prelude.isValid()) {
        const QDateTime targetHourBoundary = prelude.addMSecs(ChimeRenderer::hourPreludeMilliseconds());
        if ((!lastPlayedMelodyBoundary.isValid() || targetHourBoundary > lastPlayedMelodyBoundary)
            && playChime(targetHourBoundary)) {
            lastPlayedMelodyBoundary = targetHourBoundary;
        }
    }

    const QDateTime quarterBoundary = recentQuarterBoundary(now);
    if (quarterBoundary.isValid()
        && (!lastPlayedMelodyBoundary.isValid() || quarterBoundary > lastPlayedMelodyBoundary)
        && playChime(quarterBoundary)) {
        lastPlayedMelodyBoundary = quarterBoundary;
    }

    scheduleNextChimeEvent();
}

bool Chime::prepareAudioOutput() {
    QAudioFormat selectedFormat;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QAudioDevice device = QMediaDevices::defaultAudioOutput();
    if (device.isNull()) {
        resetAudioOutput();
        return false;
    }

    QAudioFormat requested;
    requested.setSampleRate(44100);
    requested.setChannelCount(2);
    requested.setSampleFormat(QAudioFormat::Int16);
    selectedFormat = device.isFormatSupported(requested) ? requested : device.preferredFormat();
    if (!ChimeRenderer::canRender(selectedFormat)) {
        resetAudioOutput();
        return false;
    }

    resetAudioOutput();
    if (audioFormat != selectedFormat) {
        renderedChimes.clear();
        hourStrikeData.clear();
    }

    audioFormat = selectedFormat;
    audioOutput = new QAudioSink(device, audioFormat, this);
#else
    const QAudioDeviceInfo device = QAudioDeviceInfo::defaultOutputDevice();
    if (device.isNull()) {
        resetAudioOutput();
        return false;
    }

    QAudioFormat requested;
    requested.setSampleRate(44100);
    requested.setChannelCount(2);
    requested.setSampleSize(16);
    requested.setCodec("audio/pcm");
    requested.setByteOrder(QSysInfo::ByteOrder == QSysInfo::LittleEndian
                               ? QAudioFormat::LittleEndian
                               : QAudioFormat::BigEndian);
    requested.setSampleType(QAudioFormat::SignedInt);
    selectedFormat = device.isFormatSupported(requested) ? requested : device.nearestFormat(requested);
    if (!ChimeRenderer::canRender(selectedFormat)) {
        resetAudioOutput();
        return false;
    }

    resetAudioOutput();
    if (audioFormat != selectedFormat) {
        renderedChimes.clear();
        hourStrikeData.clear();
    }

    audioFormat = selectedFormat;
    audioOutput = new QAudioOutput(device, audioFormat, this);
#endif

    audioOutput->setVolume(0.75);
    return true;
}

bool Chime::playChime(const QDateTime &boundary) {
    if (!prepareAudioOutput()) return false;

    const int minute = boundary.time().minute();
    if (!renderedChimes.contains(minute)) renderedChimes.insert(minute, ChimeRenderer::renderQuarterChime(minute, audioFormat));

    const QByteArray melody = renderedChimes.value(minute);
    if (melody.isEmpty()) return false;
    audioData = melody;
    return startPlayback();
}

bool Chime::playHourStrikes(const QDateTime &boundary) {
    if (!prepareAudioOutput()) return false;
    if (hourStrikeData.isEmpty()) hourStrikeData = ChimeRenderer::renderHourStrike(audioFormat);
    if (hourStrikeData.isEmpty()) return false;

    int strikeCount = boundary.time().hour() % 12;
    if (strikeCount == 0) strikeCount = 12;

    audioData.clear();
    audioData.reserve(strikeCount * hourStrikeData.size());
    for (int strike = 0; strike < strikeCount; ++strike) audioData.append(hourStrikeData);
    return startPlayback();
}

bool Chime::startPlayback() {
    if (audioData.isEmpty() || !audioOutput) return false;
    audioBuffer.setData(audioData);
    if (!audioBuffer.open(QIODevice::ReadOnly)) return false;
    audioOutput->start(&audioBuffer);
    return true;
}

void Chime::cacheChimes() {
    const int quarterMinutes[] = {0, 15, 30, 45};
    for (int minute : quarterMinutes) {
        if (!renderedChimes.contains(minute)) renderedChimes.insert(minute, ChimeRenderer::renderQuarterChime(minute, audioFormat));
    }
    if (hourStrikeData.isEmpty()) hourStrikeData = ChimeRenderer::renderHourStrike(audioFormat);
}

void Chime::stopPlayback() {
    if (audioOutput) audioOutput->stop();
    audioBuffer.close();
}

void Chime::resetAudioOutput() {
    stopPlayback();
    delete audioOutput;
    audioOutput = nullptr;
}