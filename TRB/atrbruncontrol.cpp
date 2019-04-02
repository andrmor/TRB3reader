#include "atrbruncontrol.h"

#include <QDebug>
#include <QObject>

bool ATrbRunControl::StartBoard()
{
    if (prBoard)
    {
        qDebug() << "Already started";
        return false;
    }

    qDebug() << "Starting up...";
    bStartLogFinished = false;
    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host) << QString("'%1'").arg(StartupScript);

    qDebug() << "Init board:"<<command << args;

    prBoard = new QProcess();
    connect(prBoard, SIGNAL(finished(int, QProcess::ExitStatus )), this, SLOT(onBoardFinished(int , QProcess::ExitStatus )));
    connect(prBoard, SIGNAL(readyRead()), this, SLOT(onReadyBoardLog()));
    prBoard->setProcessChannelMode(QProcess::MergedChannels);
    prBoard->start(command, args);

    return true;
}

void ATrbRunControl::StopBoard()
{
    if (prBoard)
    {
        qDebug() << "Stopping board";
        prBoard->terminate();
        //prStartup->close();
    }
}

bool ATrbRunControl::StartAcquire()
{
    if (!prBoard)
    {
        qDebug() << "Board not ready!";
        return false;
    }
    if (prAcquire)
    {
        qDebug() << "Already acquiring";

        return false;
    }

    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host) << QString("'/home/rpcuser/trbsoft/userscripts/trb130/acquire_Andr.sh'");
    qDebug() << "Starting acquisition:"<<command << args;

    prAcquire = new QProcess();
    connect(prAcquire, SIGNAL(finished(int, QProcess::ExitStatus )), this, SLOT(onAcquireFinished(int,QProcess::ExitStatus)));
    connect(prAcquire, SIGNAL(readyRead()), this, SLOT(onReadyAcquireLog()));
    prAcquire->setProcessChannelMode(QProcess::MergedChannels);

    prAcquire->start(command, args);

    return true;
}

void ATrbRunControl::StopAcquire()
{
    QString command = "ssh";
    QStringList args;
    //args << QString("%1@%2").arg(User).arg(Host) << "'/usr/bin/pkill -ef dabc_exe'";
    args << QString("%1@%2").arg(User).arg(Host) << "/usr/bin/pkill" << "-ef" << "dabc_exe";
    qDebug() << "Kill command:"<<command << args;
    QProcess * pr = new QProcess();
    pr->setProcessChannelMode(QProcess::MergedChannels);
    pr->start(command, args);

    if(!pr->waitForStarted(1000)){
        qDebug() << "Could not wait to start...";
    }

    if(!pr->waitForFinished(3000)) {
        qDebug() << "Could not wait to finish...";
    }

    pr->closeWriteChannel();
    qDebug() << pr->readAll();

    delete pr;
    qDebug() << "-----KILL finished";
}

void ATrbRunControl::onBoardFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Board process finished!";
    qDebug() << "Exit code:"<<exitCode;
    qDebug() << "Exit status"<< exitStatus;
    bStartLogFinished = false;
    delete prBoard; prBoard = 0;
    emit sigBoardOff();
}

void ATrbRunControl::onAcquireFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Acquire process finished!";
    qDebug() << "Exit code:"<<exitCode;
    qDebug() << "Exit status"<< exitStatus;
    delete prAcquire; prAcquire = 0;
    //emit sigAcquireOff();
}

void ATrbRunControl::onReadyBoardLog()
{
    if (bStartLogFinished)
    {
        //qDebug() << "Expecting text every second - inicates the board is alive";
        emit sigBoardIsAlive();
        return;
    }

    QString log(prBoard->readAll());
    if (log == "Starting trigger\n")
        bStartLogFinished = true;
    boardLogReady(log);
}

void ATrbRunControl::onReadyAcquireLog()
{
    QString log(prAcquire->readAll());
    qDebug() << log;
}
