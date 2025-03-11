#include "aservermonitorwindow.h"
#include "ui_aservermonitorwindow.h"
#include "mainwindow.h"
#include "anetworkmodule.h"
#include "amessage.h"

#include <QHostAddress>

AServerMonitorWindow::AServerMonitorWindow(MainWindow &MW, ANetworkModule &Network, QWidget *parent) :
    QMainWindow(parent), MW(MW), Network(Network),
    ui(new Ui::AServerMonitorWindow)
{
    ui->setupUi(this);
}

AServerMonitorWindow::~AServerMonitorWindow()
{
    delete ui;
}

void AServerMonitorWindow::appendText(const QString text)
{
    ui->pteOutput->appendPlainText(text);
}

void AServerMonitorWindow::on_pbStart_clicked()
{
    QString ip = ui->leIP->text();

    QHostAddress ha = QHostAddress(ip);
    if (ha.isNull())
    {
        message("Not valid IP address! Use, e.g., 127.0.0.1", this);
        return;
    }

    Network.StartWebSocketServer(ha, ui->leiPort->text().toInt());
}

void AServerMonitorWindow::on_pbStop_clicked()
{
    Network.StopWebSocketServer();
}

void AServerMonitorWindow::onServerstatusChanged()
{
    bool bIsRunning = Network.isWebSocketServerRunning();

    ui->pbStart->setEnabled(!bIsRunning);
    ui->pbStop->setEnabled(bIsRunning);

    MW.SetEnabled(!bIsRunning);
}

void AServerMonitorWindow::on_leIP_editingFinished()
{
    Network.StopWebSocketServer();

    QString ip = ui->leIP->text();

    QHostAddress ha = QHostAddress(ip);
    if (ha.isNull())
    {
        message("Not valid IP address! Use, e.g., 127.0.0.1", this);
        return;
    }
}

void AServerMonitorWindow::on_leiPort_editingFinished()
{
    Network.StopWebSocketServer();
}
