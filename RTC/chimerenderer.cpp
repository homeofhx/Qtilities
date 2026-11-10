#include "chimerenderer.h"
#include <QAudioFormat>
#include <QSysInfo>
#include <QVector>
#include <QtMath>
#include <cstring>

namespace {
constexpr qreal BeatSeconds = 1.0;
constexpr qreal PhraseSeconds = BeatSeconds * 5.0;
constexpr qreal BellDecaySeconds = 2.2;
constexpr qreal HourStrikeFrequency = 164.8138;
constexpr qreal HourStrikeIntervalSeconds = 4.5;
constexpr qreal HourStrikeDecaySeconds = 4.0;
constexpr qreal HourStrikePauseSeconds = 0.8;
constexpr qreal FullHourMelodySeconds = 3 * PhraseSeconds + 3 * BeatSeconds + BellDecaySeconds;
constexpr int HourPreludeMilliseconds = static_cast<int>((FullHourMelodySeconds + HourStrikePauseSeconds) * 1000 + 0.5);
constexpr qreal Pi = 3.14159265358979323846;

static_assert(HourPreludeMilliseconds < 60 * 1000, "The hourly prelude must start within :59");

constexpr qreal Changes[5][4] = {
    {415.3047, 369.9944, 329.6276, 246.9417},
    {329.6276, 415.3047, 369.9944, 246.9417},
    {329.6276, 369.9944, 415.3047, 329.6276},
    {415.3047, 329.6276, 369.9944, 246.9417},
    {246.9417, 369.9944, 415.3047, 329.6276},
};

QVector<int> changesForMinute(int minute) {
    switch (minute) {
    case 15:
        return {0};
    case 30:
        return {1, 2};
    case 45:
        return {3, 4, 0};
    case 0:
        return {1, 2, 3, 4};
    default:
        return {};
    }
}

void appendWord(QByteArray &data, quint64 word, int byteCount, bool littleEndian) {
    for (int byte = 0; byte < byteCount; ++byte) {
        const int shift = (littleEndian ? byte : byteCount - byte - 1) * 8;
        data.append(static_cast<char>((word >> shift) & 0xff));
    }
}

void appendSignedSample(QByteArray &data, qreal sample, int bits, bool littleEndian) {
    const qint64 maximum = (qint64(1) << (bits - 1)) - 1;
    const qint64 value = qRound64(qBound(qreal(-1.0), sample, qreal(1.0)) * maximum);
    appendWord(data, static_cast<quint64>(value), bits / 8, littleEndian);
}

void appendUnsignedSample(QByteArray &data, qreal sample, int bits, bool littleEndian) {
    const quint64 maximum = (quint64(1) << bits) - 1;
    const qreal normalized = (qBound(qreal(-1.0), sample, qreal(1.0)) + 1.0) / 2.0;
    appendWord(data, static_cast<quint64>(qRound64(normalized * maximum)), bits / 8, littleEndian);
}

void appendFloatSample(QByteArray &data, qreal sample, int bits, bool littleEndian) {
    if (bits == 32) {
        const float value = static_cast<float>(sample);
        quint32 word;
        std::memcpy(&word, &value, sizeof(word));
        appendWord(data, word, sizeof(word), littleEndian);
    } else {
        const double value = sample;
        quint64 word;
        std::memcpy(&word, &value, sizeof(word));
        appendWord(data, word, sizeof(word), littleEndian);
    }
}

void appendSample(QByteArray &data, qreal sample, const QAudioFormat &format) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const bool littleEndian = QSysInfo::ByteOrder == QSysInfo::LittleEndian;
    switch (format.sampleFormat()) {
    case QAudioFormat::UInt8:
        appendUnsignedSample(data, sample, 8, littleEndian);
        break;
    case QAudioFormat::Int16:
        appendSignedSample(data, sample, 16, littleEndian);
        break;
    case QAudioFormat::Int32:
        appendSignedSample(data, sample, 32, littleEndian);
        break;
    case QAudioFormat::Float:
        appendFloatSample(data, sample, 32, littleEndian);
        break;
    default:
        break;
    }
#else
    const int bits = format.sampleSize();
    const bool littleEndian = format.byteOrder() == QAudioFormat::LittleEndian;
    switch (format.sampleType()) {
    case QAudioFormat::SignedInt:
        appendSignedSample(data, sample, bits, littleEndian);
        break;
    case QAudioFormat::UnSignedInt:
        appendUnsignedSample(data, sample, bits, littleEndian);
        break;
    case QAudioFormat::Float:
        appendFloatSample(data, sample, bits, littleEndian);
        break;
    default:
        break;
    }
#endif
}

int bytesPerSample(const QAudioFormat &format) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return format.bytesPerSample();
#else
    return format.sampleSize() / 8;
#endif
}

QByteArray encodeSamples(const QVector<float> &samples, const QAudioFormat &format) {
    qreal peak = 0.0;
    for (float sample : samples) peak = qMax(peak, qAbs(qreal(sample)));
    const qreal gain = peak > 0.92 ? 0.92 / peak : 1.0;

    QByteArray rendered;
    rendered.reserve(samples.size() * format.channelCount() * bytesPerSample(format));
    for (float sample : samples) {
        const qreal adjusted = sample * gain;
        for (int channel = 0; channel < format.channelCount(); ++channel) appendSample(rendered, adjusted, format);
    }
    return rendered;
}
}

int ChimeRenderer::hourPreludeMilliseconds() {
    return HourPreludeMilliseconds;
}

bool ChimeRenderer::canRender(const QAudioFormat &format) {
    if (!format.isValid() || format.sampleRate() <= 0 || format.channelCount() <= 0) return false;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return format.sampleFormat() == QAudioFormat::UInt8
           || format.sampleFormat() == QAudioFormat::Int16
           || format.sampleFormat() == QAudioFormat::Int32
           || format.sampleFormat() == QAudioFormat::Float;
#else
    if (format.codec() != "audio/pcm") return false;
    const int bits = format.sampleSize();
    if (format.sampleType() == QAudioFormat::Float) return bits == 32 || bits == 64;
    return (format.sampleType() == QAudioFormat::SignedInt
            || format.sampleType() == QAudioFormat::UnSignedInt)
           && (bits == 8 || bits == 16 || bits == 24 || bits == 32);
#endif
}

QByteArray ChimeRenderer::renderQuarterChime(int minute, const QAudioFormat &format) {
    const QVector<int> changes = changesForMinute(minute);
    if (changes.isEmpty() || !format.isValid()) return {};

    const int sampleRate = format.sampleRate();
    const qreal duration = (changes.size() - 1) * PhraseSeconds + 3 * BeatSeconds + BellDecaySeconds;
    QVector<float> samples(qCeil(duration * sampleRate), 0.0f);
    const int bellSamples = qCeil(BellDecaySeconds * sampleRate);

    for (int phrase = 0; phrase < changes.size(); ++phrase) {
        for (int note = 0; note < 4; ++note) {
            const qreal frequency = Changes[changes[phrase]][note];
            const qreal onset = phrase * PhraseSeconds + note * BeatSeconds;
            const int startSample = qRound(onset * sampleRate);

            for (int offset = 0; offset < bellSamples && startSample + offset < samples.size(); ++offset) {
                const qreal t = qreal(offset) / sampleRate;
                const qreal attack = qMin(qreal(1.0), t / 0.006);
                const qreal release = qMin(qreal(1.0), (BellDecaySeconds - t) / 0.08);
                const qreal bell = 0.58 * qSin(2 * Pi * frequency * t) * qExp(-1.25 * t)
                                   + 0.24 * qSin(2 * Pi * frequency * 2.01 * t) * qExp(-1.75 * t)
                                   + 0.12 * qSin(2 * Pi * frequency * 3.00 * t) * qExp(-2.25 * t)
                                   + 0.06 * qSin(2 * Pi * frequency * 4.10 * t) * qExp(-2.85 * t);
                samples[startSample + offset] += static_cast<float>(0.42 * attack * release * bell);
            }
        }
    }
    return encodeSamples(samples, format);
}

QByteArray ChimeRenderer::renderHourStrike(const QAudioFormat &format) {
    if (!format.isValid()) return {};

    const int sampleRate = format.sampleRate();
    QVector<float> samples(qCeil(HourStrikeIntervalSeconds * sampleRate), 0.0f);
    for (int offset = 0; offset < samples.size(); ++offset) {
        const qreal t = qreal(offset) / sampleRate;
        if (t >= HourStrikeDecaySeconds) continue;
        const qreal attack = qMin(qreal(1.0), t / 0.008);
        const qreal release = qMin(qreal(1.0), (HourStrikeDecaySeconds - t) / 0.15);
        const qreal bell = 0.58 * qSin(2 * Pi * HourStrikeFrequency * t) * qExp(-0.75 * t)
                           + 0.24 * qSin(2 * Pi * HourStrikeFrequency * 2.01 * t) * qExp(-1.1 * t)
                           + 0.12 * qSin(2 * Pi * HourStrikeFrequency * 2.74 * t) * qExp(-1.5 * t)
                           + 0.06 * qSin(2 * Pi * HourStrikeFrequency * 4.12 * t) * qExp(-2.0 * t);
        samples[offset] = static_cast<float>(0.52 * attack * release * bell);
    }
    return encodeSamples(samples, format);
}