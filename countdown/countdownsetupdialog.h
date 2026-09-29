#ifndef COUNTDOWNSETUPDIALOG_H
#define COUNTDOWNSETUPDIALOG_H

#include <QDialog>

class QLineEdit;
class QLabel;

class CountdownSetupDialog : public QDialog {
    Q_OBJECT

public:
    explicit CountdownSetupDialog(QWidget *parent = nullptr);

    qint64 durationSeconds() const { return parsedDurationSeconds; }
    QString countdownLabel() const;

public slots:
    void accept() override;

private:
    void showWarning(const QString &message);
    void clearWarning();

    QLineEdit *hourEdit = nullptr;
    QLineEdit *minuteEdit = nullptr;
    QLineEdit *secondEdit = nullptr;
    QLineEdit *labelEdit = nullptr;
    QLabel *warningLabel = nullptr;
    qint64 parsedDurationSeconds = 0;
};

#endif