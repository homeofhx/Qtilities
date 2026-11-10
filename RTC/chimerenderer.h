#ifndef CHIMERENDERER_H
#define CHIMERENDERER_H
#include <QByteArray>

class QAudioFormat;

namespace ChimeRenderer {

int hourPreludeMilliseconds();
bool canRender(const QAudioFormat &format);
QByteArray renderQuarterChime(int minute, const QAudioFormat &format);
QByteArray renderHourStrike(const QAudioFormat &format);

}

#endif