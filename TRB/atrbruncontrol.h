#ifndef ATRBRUNCONTROL_H
#define ATRBRUNCONTROL_H

#include <QObject>
#include <QProcess>

class MasterConfig;
class ATrbRunSettings;

class ATrbRunControl : public QObject
{
    Q_OBJECT
public:
    ATrbRunControl(MasterConfig & settings, const QString & exchangeDir);

    const QString StartBoard();
    void StopBoard();

    bool isBoardDisconnected() const {return (ConnectStatus == Disconnected);}
    void RestartBoard();

    const QString StartAcquire(); //returns error string, empty if all is ok
    void StopAcquire();

    const QString updateXML();

    const QString updateCTSsetupScript();
    const QString updateBufferSetupScript();
    const QString sendCTStoTRB();
    const QString sendBufferControlToTRB();
    const QString readBufferControlFromTRB();

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
    MasterConfig & Settings;
    const ATrbRunSettings & RunSettings;
    const QString sExchangeDir;
    QProcess * prBoard = 0;
    QProcess * prAcquire = 0;

    enum eConnectStatus {Disconnected, Connecting, WaitingFirstReply, Connected};
    eConnectStatus ConnectStatus = Disconnected;

public:
    const QString & Host;
    const QString & User;

    int StatEvents = 0;
    double StatRate = 0;
    double StatData = 0;
    QString StatDataUnits;

private:
    const QString sshCopyFileToHost(const QString & localFileName, const QString & hostDir);  // returns error message, empty if success
    const QString sshCopyFileFromHost(const QString & hostFileName, const QString & localDir);  // returns error message, empty if success
    const QString makeDirOnHost(const QString & hostDir);  // returns error message, empty if success

    const QString sendCommandToHost(const QString & command);

    const QStringList bufferRecordsToCommands();
};

#endif // ATRBRUNCONTROL_H
