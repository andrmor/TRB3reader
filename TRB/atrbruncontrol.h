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
    int HldFileSize;
    QString StorageXML;

    int StatEvents = 0;
    double StatRate = 0;
    double StatData = 0;
    QString StatDataUnits;

    bool StartBoard();
    void StopBoard();

    const QString StartAcquire(); //returns error string, empty if all is ok
    void StopAcquire();

    const QString updateXML();

private slots:
    void onBoardFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onAcquireFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void onReadyBoardLog();
    void onReadyAcquireLog();

signals:
    //void sigBoardOn();
    void sigBoardOff();

    void sigBoardIsAlive();
    void sigAcquireIsAlive();

    void boardLogReady(const QString txt);


private:
    QProcess * prBoard = 0;
    QProcess * prAcquire = 0;
    bool bStartLogFinished = false;

    QString sExchangeDir;

private:
    const QString sshCopyFileToHost(const QString & localFileName, const QString & hostDir);  // returns error message, empty if success
    const QString sshCopyFileFromHost(const QString & hostFileName, const QString & localDir);  // returns error message, empty if success
    const QString makeDirOnHost(const QString & hostDir);  // returns error message, empty if success

};

#endif // ATRBRUNCONTROL_H
