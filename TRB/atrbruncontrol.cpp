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

const QString ATrbRunControl::sshCopyFileToHost(const QString &localFileName, const QString &hostDir)
{
    QString command = "scp";
    QStringList args;
    args << localFileName << QString("%1@%2:%3").arg(User).arg(Host).arg(hostDir) ;
    qDebug() << "Transfer command:"<<command << args;
    QProcess pr;
    //pr.setProcessChannelMode(QProcess::MergedChannels);
    pr.start(command, args);

    if (!pr.waitForStarted(500)) return "Could not start copy process";
    if (!pr.waitForFinished(2000)) return "Timeout on copy to host";

    //pr.closeWriteChannel();
    //qDebug() << pr.readAll();
    return "";
}

const QString ATrbRunControl::sshCopyFileFromHost(const QString & hostFileName, const QString & localDir)
{
    QString command = "scp";
    QStringList args;
    args << QString("%1@%2:%3").arg(User).arg(Host).arg(hostFileName) << localDir;
    qDebug() << "Transfer command:"<<command << args;
    QProcess pr;
    //pr.setProcessChannelMode(QProcess::MergedChannels);
    pr.start(command, args);

    if (!pr.waitForStarted(500)) return "Could not start copy process";
    if (!pr.waitForFinished(2000)) return "Timeout on copy from host";

    //pr.closeWriteChannel();
    //qDebug() << pr.readAll();
    return "";
}

#include <QXmlStreamReader>
#include <QFileInfo>
void ATrbRunControl::updateXML(const QString & NewDir, int NewSizeMb)
{
    QString where = sExchangeDir;
    if (!where.endsWith('/')) where += '/';

    QString err = sshCopyFileFromHost(StorageXML, where);
    if (!err.isEmpty())
    {
        qDebug() << err;
        return;
    }

    QFileInfo hostFileInfo(StorageXML);
    QString localFileName = where + hostFileInfo.fileName();
    QString hostDir = hostFileInfo.absolutePath();

    qDebug() << "On host:" << hostDir << localFileName;

    QString newXml;
    QFile file(localFileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        int NumComment = 0;
        while (!file.atEnd())
        {
            QString line = file.readLine();
            //qDebug() << line;
            NumComment += line.count("<!--");
            NumComment -= line.count("-->");
            if (!line.contains("<!--") && NumComment == 0)
            {
                if (line.contains("Output1"))
                {
                    qDebug() << "Found!";
                    //<OutputPort name="Output1" url="hld://${HOME}/hlds/dabc.hld?maxsize=10"/>
                    //<OutputPort name="Output1" url="hld:///media/externalDrive/data/hlds/dabc.hld?maxsize=10"/>
                    QString dir = NewDir;
                    if (!dir.endsWith('/')) dir += '/';
                    line = QString("<OutputPort name=\"Output1\" url=\"hld://%1dabc.hld?maxsize=%2\"/>").arg(dir).arg(NewSizeMb);
                }
            }
            newXml += line;
        }
    }
    else qDebug() << "Cannot open file" << localFileName;
    file.close();

    //QFile fout(fName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QByteArray ar = newXml.toLocal8Bit().data();
        file.write(ar);
    }
    else qDebug() << "Cannot write";
    file.close();

    err = sshCopyFileToHost(localFileName, hostDir);
    if (!err.isEmpty())
    {
        qDebug() << err;
        return;
    }

}
