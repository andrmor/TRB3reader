#ifndef ATRBRUNCONTROL_H
#define ATRBRUNCONTROL_H

#include <QObject>
#include <QProcess>

class ATrbRunControl : public QObject
{
    Q_OBJECT
public:
    ATrbRunControl(){}

    QString Host;
    QString User;
    QString StartupScript;

    bool StartBoard();
    void StopBoard();

    bool StartAcquire();
    void StopAcquire();

private slots:
    void onBoardFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onAcquireFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void onReadyBoardLog();
    void onReadyAcquireLog();

signals:
    void sigBoardOn();
    void sigBoardOff();
    void sigBoardIsAlive();

    void boardLogReady(const QString txt);

private:
    QProcess * prBoard = 0;
    QProcess * prAcquire = 0;
    bool bStartLogFinished = false;

};

#endif // ATRBRUNCONTROL_H
