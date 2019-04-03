#include "atrbruncontrol.h"

#include <QDebug>
#include <QObject>
#include <QElapsedTimer>

ATrbRunControl::ATrbRunControl(const QString &exchangeDir)
{
    sExchangeDir = exchangeDir;
}

bool ATrbRunControl::StartBoard()
{
    if (prBoard)
    {
        qDebug() << "Already started";
        return false;
    }

    bStartLogFinished = false;

    qDebug() << "Starting up...";
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

    StatEvents = 0;
    StatRate = 0;
    StatData = 0;



    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host) << AcquireScript;
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
    if (log == "Starting CTS\n")
        bStartLogFinished = true;
    boardLogReady(log);
}

void ATrbRunControl::onReadyAcquireLog()
{
    QString txt = prAcquire->readAll();
    QString log(txt);

    if (txt.startsWith("---------------------------------------------\nEvents"))
    {
        QStringList f = log.split('\n');
        if (f.size()>2)
        {
            QString st = f.at(1);
            QStringList ff = st.split(' ', QString::SkipEmptyParts);
            if (ff.size() > 7)
            {
                StatEvents = ff.at(1).toInt();
                StatRate = ff.at(3).toDouble();
                StatData = ff.at(6).toDouble();
                StatDataUnits = ff.at(7);
            }
        }
    }

    //qDebug() << log;
    emit sigAcquireIsAlive();
}

void ATrbRunControl::updateXML()
{
    QString command = "scp";
    QStringList args;
    //args << QString("%1@%2:%3").arg(User).arg(Host).arg(StorageXML) << sExchangeDir;
    args << QString("%1@%2:%3").arg(User).arg(Host).arg(StorageXML) << "/home/andr/tmp";
    qDebug() << "Transfer command:"<<command << args;
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
    qDebug() << "-----Transfer finished";
}
