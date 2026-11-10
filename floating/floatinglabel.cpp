#include "floatinglabel.h"

#include <QGraphicsDropShadowEffect>
#include <QLabel>

namespace {
constexpr int DisplayWidth = 500;
constexpr int HeadlineFontSize = 50;
constexpr int DetailFontSize = 32;
}

QLabel *FloatingUi::createLabel(QWidget *parent, const QString &color) {
    QLabel *label = new QLabel(parent);
    label->setFixedWidth(DisplayWidth);
    label->setAlignment(Qt::AlignCenter);
    label->setTextFormat(Qt::RichText);
    setLabelColor(label, color);

    QGraphicsDropShadowEffect *outline = new QGraphicsDropShadowEffect(label);
    outline->setBlurRadius(30);
    outline->setColor(Qt::white);
    outline->setOffset(0);
    label->setGraphicsEffect(outline);
    return label;
}

void FloatingUi::setLabelColor(QLabel *label, const QString &color) {
    label->setStyleSheet(QStringLiteral("color: %1;"
                                        "font-family: 'Consolas', 'Menlo', monospace;"
                                        "font-size: %2px;"
                                        "font-weight: bold;"
                                        "background: transparent;")
                             .arg(color)
                             .arg(HeadlineFontSize));
}

QString FloatingUi::formatText(const QString &headline, const QString &detail) {
    return QStringLiteral("%1<br><span style='font-size: %2px; font-weight: normal;'>%3</span>")
        .arg(headline.toHtmlEscaped())
        .arg(DetailFontSize)
        .arg(detail.toHtmlEscaped());
}
