#ifndef ATRBRUNCONTROL_H
#define ATRBRUNCONTROL_H

#include <QObject>
#include <QProcess>

class MasterConfig;
class ATrbRunSettings;
class ANetworkModule;
class QTimer;

class ATrbRunControl : public QObject
{
    Q_OBJECT
public:
    ATrbRunControl(MasterConfig & settings, ANetworkModule & Network, const QString & exchangeDir);

    const QString StartBoard();
    void StopBoard();

    bool isAcquireProcessExists() const {return prAcquire;}
    bool isBoardProcessExists() const {return prBoard;}
    void RestartBoard();

    QString StartAcquire(); //returns error string, empty if all is ok
    void StopAcquire();

    const QString updateXML();

    QString updateCTSsetupScript();
    QString updateBufferSetupScript();
    QString sendCTStoTRB();
    QString ReadTriggerSettingsFromBoard();
    QString sendTriggerLogicToTRB();
    QString readTriggerLogicFromTRB();
    QString sendBufferControlToTRB();
    QString readBufferControlFromTRB();

    QString sendTimeSettingsToTRB();
    QString readTimeSettingsFromTRB();

    QString sendTriggerGainsToBoard();

    void  checkFreeSpace();

private slots:
    void onBoardFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onAcquireFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void onReadyBoardLog();
    void onReadyAcquireLog();

    void onFreeSpaceCheckerFinished();
    void onFreeSpaceCheckerReady();
    void onFreeSpaceCheckerTimeout();

signals:
    //void sigBoardOn();
    void sigBoardOff();
    void sigAcquireOff();

    void sigBoardIsAlive(double acceptedRate);
    void sigAcquireIsAlive();

    void boardLogReady(const QString txt);
    void requestClearLog();

    void freeSpaceCheckReady(long bytes); //can be -1 for n.a.

private:    
    MasterConfig & Settings;
    ATrbRunSettings & RunSettings;
    ANetworkModule & Network;
    const QString sExchangeDir;
    QProcess * prBoard = 0;
    QProcess * prAcquire = 0;

    QProcess * prFreeSpaceChecker = 0;
    QTimer * timerFreeSpaceChecker = 0;
    long lastFreeSpace = -1;

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
    void  recallConfiguration(); //executed after board is connected !!!

    const QString sshCopyFileToHost(const QString & localFileName, const QString & hostDir);  // returns error message, empty if success
    const QString sshCopyFileFromHost(const QString & hostFileName, const QString & localDir);  // returns error message, empty if success
    const QString makeDirOnHost(const QString & hostDir);  // returns error message, empty if success

    const QString sendCommandToHost(const QString & command);

    const QStringList bufferRecordsToCommands();
    const QStringList CtsSettingsToCommands(bool bIncludeHidden);
    QString formTriggerGainText();
};

#endif // ATRBRUNCONTROL_H
