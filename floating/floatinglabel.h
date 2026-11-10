#ifndef FLOATINGLABEL_H
#define FLOATINGLABEL_H

#include <QString>

class QLabel;
class QWidget;

namespace FloatingUi {

QLabel *createLabel(QWidget *parent, const QString &color);
void setLabelColor(QLabel *label, const QString &color);
QString formatText(const QString &headline, const QString &detail);

}

#endif
