#ifndef AEDITCHANNELSDIALOG_H
#define AEDITCHANNELSDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class AEditChannelsDialog;
}

class AEditChannelsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AEditChannelsDialog(QString Title, QString OldText, QString Example, QWidget *parent = 0);
    ~AEditChannelsDialog();

    const QString GetText() const;

private slots:
    void on_pbAccept_clicked();
    void on_pbCancel_clicked();

private:
    Ui::AEditChannelsDialog *ui;
};

#endif // AEDITCHANNELSDIALOG_H
