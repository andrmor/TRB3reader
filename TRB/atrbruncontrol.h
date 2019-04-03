#ifndef ATRBRUNCONTROL_H
#define ATRBRUNCONTROL_H

#include <QObject>
#include <QProcess>

class ATrbRunControl : public QObject
{
    Q_OBJECT
public:
    ATrbRunControl(const QString & exchangeDir);

    QString Host;
    QString User;
    QString StartupScript;
    QString AcquireScript;

    QString HldFolder;
    double HildFileSize;
    QString StorageXML;

    int StatEvents = 0;
    double StatRate = 0;
    double StatData = 0;
    QString StatDataUnits;

    bool StartBoard();
    void StopBoard();

    bool StartAcquire();
    void StopAcquire();

    void updateXML();

private slots:
    void onBoardFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onAcquireFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void onReadyBoardLog();
    void onReadyAcquireLog();

signals:
    void sigBoardOn();
    void sigBoardOff();

    void sigBoardIsAlive();
    void sigAcquireIsAlive();

    void boardLogReady(const QString txt);


private:
    QProcess * prBoard = 0;
    QProcess * prAcquire = 0;
    bool bStartLogFinished = false;

    QString sExchangeDir;

};

#endif // ATRBRUNCONTROL_H
