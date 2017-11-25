#include "aeditchannelsdialog.h"
#include "ui_aeditchannelsdialog.h"

AEditChannelsDialog::AEditChannelsDialog(QString Title, QString OldText, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AEditChannelsDialog)
{
    ui->setupUi(this);
    ui->labTitle->setText(Title);
    ui->pteEdit->appendPlainText(OldText);
}

AEditChannelsDialog::~AEditChannelsDialog()
{
    delete ui;
}

const QString AEditChannelsDialog::GetText() const
{
    return ui->pteEdit->document()->toPlainText();
}

void AEditChannelsDialog::on_pbAccept_clicked()
{
    accept();
}

void AEditChannelsDialog::on_pbCancel_clicked()
{
    reject();
}
