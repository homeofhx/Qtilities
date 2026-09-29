#include "countdownsetupdialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include <limits>

CountdownSetupDialog::CountdownSetupDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Set Up Timer");
    setModal(true);

    QFormLayout *layout = new QFormLayout(this);

    hourEdit = new QLineEdit("00", this);
    hourEdit->setObjectName("countdownHours");
    hourEdit->setAlignment(Qt::AlignCenter);
    hourEdit->setFixedWidth(35);
    hourEdit->setPlaceholderText("hh");
    hourEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d+"), hourEdit));

    minuteEdit = new QLineEdit("00", this);
    minuteEdit->setObjectName("countdownMinutes");
    minuteEdit->setAlignment(Qt::AlignCenter);
    minuteEdit->setFixedWidth(35);
    minuteEdit->setMaxLength(2);
    minuteEdit->setPlaceholderText("mm");
    minuteEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d+"), minuteEdit));

    secondEdit = new QLineEdit("00", this);
    secondEdit->setObjectName("countdownSeconds");
    secondEdit->setAlignment(Qt::AlignCenter);
    secondEdit->setFixedWidth(35);
    secondEdit->setMaxLength(2);
    secondEdit->setPlaceholderText("ss");
    secondEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("\\d+"), secondEdit));

    QHBoxLayout *timeLayout = new QHBoxLayout();
    timeLayout->setContentsMargins(0, 0, 0, 0);
    timeLayout->setSpacing(6);
    timeLayout->addWidget(hourEdit);
    timeLayout->addWidget(new QLabel(":", this));
    timeLayout->addWidget(minuteEdit);
    timeLayout->addWidget(new QLabel(":", this));
    timeLayout->addWidget(secondEdit);
    timeLayout->addStretch();
    layout->addRow("Duration:", timeLayout);

    labelEdit = new QLineEdit(this);
    labelEdit->setObjectName("countdownLabel");
    labelEdit->setPlaceholderText("Countdown label");
    layout->addRow("Label:", labelEdit);

    warningLabel = new QLabel(this);
    warningLabel->setObjectName("countdownWarning");
    warningLabel->setStyleSheet("color: #D32F2F;");
    warningLabel->setWordWrap(true);
    warningLabel->hide();
    layout->addRow(warningLabel);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &CountdownSetupDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &CountdownSetupDialog::reject);
    layout->addRow(buttons);

    connect(hourEdit, &QLineEdit::textChanged, this, &CountdownSetupDialog::clearWarning);
    connect(minuteEdit, &QLineEdit::textChanged, this, &CountdownSetupDialog::clearWarning);
    connect(secondEdit, &QLineEdit::textChanged, this, &CountdownSetupDialog::clearWarning);
    connect(labelEdit, &QLineEdit::textChanged, this, &CountdownSetupDialog::clearWarning);

    labelEdit->setFocus();
}

QString CountdownSetupDialog::countdownLabel() const {
    return labelEdit->text().trimmed();
}

void CountdownSetupDialog::accept() {
    parsedDurationSeconds = 0;

    bool hoursOk = false;
    bool minutesOk = false;
    bool secondsOk = false;
    const qint64 hours = hourEdit->text().trimmed().toLongLong(&hoursOk);
    int minutes = minuteEdit->text().trimmed().toInt(&minutesOk);
    int seconds = secondEdit->text().trimmed().toInt(&secondsOk);

    if (!hoursOk || !minutesOk || !secondsOk || hours < 0 || minutes < 0 || seconds < 0) {
        showWarning("Enter hour/minute/second!");
        return;
    }

    minutes = qMin(minutes, 59);
    seconds = qMin(seconds, 59);
    minuteEdit->setText(QString::number(minutes).rightJustified(2, '0'));
    secondEdit->setText(QString::number(seconds).rightJustified(2, '0'));

    if (hours > (std::numeric_limits<qint64>::max() / 1000 - 3599) / 3600) {
        showWarning("Duration is too large!");
        return;
    }

    parsedDurationSeconds = hours * 3600 + minutes * 60 + seconds;
    if (parsedDurationSeconds <= 0) {
        showWarning("Invalid duration!");
        return;
    }

    if (countdownLabel().isEmpty()) {
        showWarning("Enter a label!");
        return;
    }

    QDialog::accept();
}

void CountdownSetupDialog::showWarning(const QString &message) {
    warningLabel->setText(message);
    warningLabel->show();
}

void CountdownSetupDialog::clearWarning() {
    warningLabel->clear();
    warningLabel->hide();
}