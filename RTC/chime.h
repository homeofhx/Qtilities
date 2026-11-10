#ifndef CHIME_H
#define CHIME_H
#include <QAudioFormat>
#include <QBuffer>
#include <QByteArray>
#include <QDateTime>
#include <QHash>
#include <QObject>
#include <QTimer>
#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QAudioSink;
using ChimeAudioOutput = QAudioSink;
#else
class QAudioOutput;
using ChimeAudioOutput = QAudioOutput;
#endif

class Chime : public QObject {
    Q_OBJECT

public:
    explicit Chime(QObject *parent = nullptr);
    ~Chime() override;

    bool isEnabled() const { return enabled; }
    void setEnabled(bool enabled);

private:
    void scheduleNextChimeEvent();
    void onSchedulerTimeout();
    bool playChime(const QDateTime &boundary);
    bool playHourStrikes(const QDateTime &boundary);
    bool startPlayback();
    bool prepareAudioOutput();
    void cacheChimes();
    void stopPlayback();
    void resetAudioOutput();

    bool enabled = false;
    QTimer schedulerTimer;
    QDateTime lastPlayedMelodyBoundary;
    QDateTime lastPlayedStrikeBoundary;

    QAudioFormat audioFormat;
    ChimeAudioOutput *audioOutput = nullptr;
    QHash<int, QByteArray> renderedChimes;
    QByteArray hourStrikeData;
    QByteArray audioData;
    QBuffer audioBuffer;
};

#endif