#ifndef ASERVERMONITORWINDOW_H
#define ASERVERMONITORWINDOW_H

#include <QMainWindow>

namespace Ui {
class AServerMonitorWindow;
}

class MainWindow;
class ANetworkModule;

class AServerMonitorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AServerMonitorWindow(MainWindow& MW, ANetworkModule& Network, QWidget *parent = 0);
    ~AServerMonitorWindow();

public slots:
    void appendText(const QString text);

private slots:
    void on_pbStart_clicked();
    void on_pbStop_clicked();
    void on_leIP_editingFinished();
    void on_leiPort_editingFinished();

public slots:
    void onServerstatusChanged();

private:
    MainWindow& MW;
    ANetworkModule& Network;
    Ui::AServerMonitorWindow *ui;

};

#endif // ASERVERMONITORWINDOW_H
