#include "atrbruncontrol.h"
#include "atrbrunsettings.h"

#include <QDebug>
#include <QObject>
#include <QElapsedTimer>

ATrbRunControl::ATrbRunControl(ATrbRunSettings & settings, const QString & exchangeDir) :
    QObject(), Settings(settings),
    Host(settings.Host), User(settings.User),
    sExchangeDir(exchangeDir) {}

const QString ATrbRunControl::StartBoard()
{
    if (prBoard) return "Already started";

    ConnectStatus = Connecting;

    qDebug() << "Starting up...";
    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host) << QString("'%1%2'").arg(Settings.getScriptDir()).arg(Settings.StartupScriptOnHost);

    qDebug() << "Init board:"<<command << args;

    prBoard = new QProcess();
    connect(prBoard, SIGNAL(finished(int, QProcess::ExitStatus )), this, SLOT(onBoardFinished(int , QProcess::ExitStatus )));
    connect(prBoard, SIGNAL(readyRead()), this, SLOT(onReadyBoardLog()));
    prBoard->setProcessChannelMode(QProcess::MergedChannels);
    prBoard->start(command, args);

    return "";
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

const QString ATrbRunControl::StartAcquire()
{
    if (!prBoard) return "Board not ready!";
    if (prAcquire) return "Acquisition is already running";

    QString err = updateXML();
    if (!err.isEmpty())
        return "Failed to update file storage settings on host:\n" + err;

    StatEvents = 0;
    StatRate = 0;
    StatData = 0;

    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host) << QString("'%1%2'").arg(Settings.getScriptDir()).arg(Settings.AcquireScriptOnHost);

    qDebug() << "Starting acquisition:"<<command << args;

    prAcquire = new QProcess();
    connect(prAcquire, SIGNAL(finished(int, QProcess::ExitStatus )), this, SLOT(onAcquireFinished(int,QProcess::ExitStatus)));
    connect(prAcquire, SIGNAL(readyRead()), this, SLOT(onReadyAcquireLog()));
    prAcquire->setProcessChannelMode(QProcess::MergedChannels);

    prAcquire->start(command, args);

    return "";
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
    ConnectStatus = Disconnected;
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
    QString log(prBoard->readAll());
    //qDebug() << "=========================";
    //qDebug() << log;
    //qDebug() << "=========================";

    switch (ConnectStatus)
    {
    case Disconnected:
        break;
    case Connecting:
        if (log == '\n') return;
        if (log == "Starting CTS\n")
            ConnectStatus = WaitingFirstReply;
        boardLogReady(log);
        break;
    case WaitingFirstReply:
        if (log.contains("Label                                | Register"))
        {
            ConnectStatus = Connected;
            emit sigBoardIsAlive();
        }
        break;
    case Connected:
        emit sigBoardIsAlive(); //"Expecting text every second - inicates the board is alive";
        break;
    }
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

const QString ATrbRunControl::makeDirOnHost(const QString & hostDir)
{
    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host) << "mkdir" << "-p" << hostDir;
    qDebug() << "mkdir command:"<<command << args;

    QProcess pr;
    //pr.setProcessChannelMode(QProcess::MergedChannels);
    pr.start(command, args);

    if (!pr.waitForStarted(500)) return "Could not start mkdir process";
    if (!pr.waitForFinished(2000)) return "Timeout on mkdir on host";

    //pr.closeWriteChannel();
    //qDebug() << pr.readAll();
    return "";
}

#include <QXmlStreamReader>
#include <QFileInfo>
const QString ATrbRunControl::updateXML()
{
    QString where = sExchangeDir;
    if (!where.endsWith('/')) where += '/';

    QString err = sshCopyFileFromHost( QString("%1%2").arg(Settings.getScriptDir()).arg(Settings.StorageXMLOnHost), where);
    if (!err.isEmpty()) return err;

    QString localFileName = where + Settings.StorageXMLOnHost;
    QString hostDir = Settings.getScriptDir();

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
                    QString dir = Settings.HldDirOnHost;
                    if (!dir.endsWith('/')) dir += '/';
                    line = QString("<OutputPort name=\"Output1\" url=\"hld://%1dabc.hld?maxsize=%2\"/>\n").arg(dir).arg(Settings.MaxHldSizeMb);
                }
            }
            newXml += line;
        }
    }
    else return "Cannot open file" + localFileName;
    file.close();

    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QByteArray ar = newXml.toLocal8Bit().data();
        file.write(ar);
    }
    else return "Cannot write to " + localFileName;
    file.close();

    err = sshCopyFileToHost(localFileName, hostDir);
    if (!err.isEmpty()) return err;

    //making dir on host
    err = makeDirOnHost(Settings.HldDirOnHost);
    if (!err.isEmpty()) return "Failed to create target folder on host";

    return "";
}

#include "afiletools.h"
const QString ATrbRunControl::updateCTSsetupScript()
{
    if (Settings.CtsControl.isEmpty())
        return "Error: CTS settings are absent in configuration";

    QString where = sExchangeDir;
    if (!where.endsWith('/')) where += '/';
    QString localCtsFileName = where + "TriggerSetup_Andr.sh";

    bool bOK = SaveTextToFile(localCtsFileName, Settings.CtsControl);
    if (!bOK) return "Cannot save CTS settings to file " + localCtsFileName;

    QFileInfo hostFileInfo(Settings.StorageXMLOnHost);
    QString hostDir = hostFileInfo.absolutePath();

    qDebug() << "On host:" << hostDir << localCtsFileName;

    QString err = sshCopyFileToHost(localCtsFileName, hostDir);
    if (!err.isEmpty()) return err;

    return "";
}

const QString ATrbRunControl::sendCTStoTRB()
{
    QStringList txt;
    txt << "trbcmd setbit 0xc001 0xa00c 0x80000000\n";
    txt << "trbcmd w 0xc001 0xa101 0xffff0040\n";
    txt << "trbcmd clearbit 0xc001 0xa00c 0x80000000\n";

    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host);

    QProcess pr;
    pr.setProcessChannelMode(QProcess::MergedChannels);
    pr.start(command, args);

    if (!pr.waitForStarted(500)) return "Could not start send";

    for (const QString & s : txt)
        pr.write(QByteArray(s.toLocal8Bit().data()));

    pr.closeWriteChannel();
    if (!pr.waitForFinished(2000)) return "Timeout on attempt to execute CTS configuration";

    QString reply1 = pr.readAll();

    pr.close();

    qDebug() << reply1;
    return "";
}

/*
const QString ATrbRunControl::sendCTStoTRB()
{
    //if (Settings.CtsControl.isEmpty()) return "Error: CTS settings are absent in configuration";

    const QString tmpFileName = "tmp.sh";

    QFileInfo hostFileInfo(Settings.StorageXMLOnHost);

    QString hostDir = Settings.getScriptDir();

    //QString txt = "#!/bin/bash\n";
    //txt += Settings.CtsControl;
    //bool bOK = SaveTextToFile(localCtsFileName, txt);
    //if (!bOK) return "Cannot save CTS settings to file " + localCtsFileName;

    qDebug() << "Dir on host:" << hostDir << "local file:"<<localCtsFileName;

    QString err = sshCopyFileToHost(localCtsFileName, hostDir);
    if (!err.isEmpty()) return err;

    //executing the script

    QStringList txt;
    //txt << "export PATH=${HOME}/trbsoft/trbnettools/bin:${PATH}\n";
       //txt << "export PATH=${PATH}:${HOME}/trbsoft/daqdata/bin\n";
       //txt << "export LD_LIBRARY_PATH=$HOME/trbsoft/trbnettools/liblocal/\n";
       //txt << "export PERL5LIB=~/trbsoft/daqtools/perllibs\n";
    //txt << "export DAQOPSERVER=localhost:1\n";

    txt << "trbcmd setbit 0xc001 0xa00c 0x80000000\n";
    txt << "trbcmd w 0xc001 0xa101 0xffff0040\n";
    txt << "trbcmd clearbit 0xc001 0xa00c 0x80000000\n";

    QString command = "ssh";
    QStringList args;
    //args << QString("%1@%2").arg(User).arg(Host) << QString("'%1/%2'").arg(hostDir).arg(tmpFileName);
    //args << QString("%1@%2").arg(User).arg(Host) << "bash" << "-c" << QString("'%1/%2'").arg(hostDir).arg(tmpFileName);
    //args << QString("%1@%2").arg(User).arg(Host) << "bash" << "-s" << "<" << QString("'%1/%2'").arg(hostDir).arg(tmpFileName);
    //args << QString("%1@%2").arg(User).arg(Host) << "bash" << "-s" << "<" << "/home/andr/.config/TRBreader/tmp.sh";

    //args << "-t" << QString("%1@%2").arg(User).arg(Host);
    args << QString("%1@%2").arg(User).arg(Host);

    QProcess pr;
    pr.setProcessChannelMode(QProcess::MergedChannels);
    pr.start(command, args);

    if (!pr.waitForStarted(500)) return "Could not start send";

    for (const QString & s : txt)
        pr.write(QByteArray(s.toLocal8Bit().data()));

    pr.closeWriteChannel();

    if (!pr.waitForFinished(2000)) return "Timeout on attempt to execute CTS configuration";

    QString reply1 = pr.readAll();

    pr.close();

    qDebug() << reply1;
    return "";

//    QString command = "cat";
//    //cat tmp.sh | ssh rpcuser@192.168.3.214 "/bin/bash"
//    QStringList args;
//    //args << localCtsFileName << "|" << "ssh" << QString("%1@%2").arg(User).arg(Host) << "\"/bin/bash\"";
//    args << localCtsFileName << "|" << "ssh" << QString("%1@%2").arg(User).arg(Host) << "\"/bin/bash\"";


//    qDebug() << "execute command:"<<command << args;

//    QProcess pr;
//    pr.setProcessChannelMode(QProcess::MergedChannels);
//    pr.start(command, args);

//    if (!pr.waitForStarted(500)) return "Could not start send";
//    if (!pr.waitForFinished(2000)) return "Timeout on attempt to execute CTS configuration";

//    pr.closeWriteChannel();
//    QString reply = pr.readAll();
//    qDebug() << reply;
//    //if (reply.contains("No such file or directory")) return "Cannot find temporary file with CTS script on host";
//    //if (reply.contains("Permission denied")) return "Cannot start temporary file with CTS script on host";

//    return "";
}
*/
