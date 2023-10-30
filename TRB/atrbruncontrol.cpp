#include "atrbruncontrol.h"
#include "masterconfig.h"
#include "atrbrunsettings.h"
#include "anetworkmodule.h"

#include <QDebug>
#include <QObject>
#include <QElapsedTimer>
#include <QApplication>
#include <QThread>
#include <QTimer>

ATrbRunControl::ATrbRunControl(MasterConfig & settings, ANetworkModule &Network, const QString & exchangeDir) :
    QObject(), Settings(settings), RunSettings(settings.TrbRunSettings),
    Network(Network), sExchangeDir(exchangeDir),
    Host(settings.TrbRunSettings.Host), User(settings.TrbRunSettings.User)
{
    timerFreeSpaceChecker = new QTimer(this);
    timerFreeSpaceChecker->setSingleShot(true);
    QObject::connect(timerFreeSpaceChecker, &QTimer::timeout, this, &ATrbRunControl::onFreeSpaceCheckerTimeout);
}

const QString ATrbRunControl::StartBoard()
{
    if (prBoard) return "Already started";

    ConnectStatus = Connecting;

    qDebug() << "Starting up...";
    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host) << QString("'%1%2'").arg(RunSettings.getScriptDir()).arg(RunSettings.StartupScriptOnHost);

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

void ATrbRunControl::RestartBoard()
{
    if (prAcquire) return;
    if (prBoard) return;

    ConnectStatus = Disconnected;

    emit requestClearLog();
    emit boardLogReady("\n\nPowering OFF...");
    QString err = sendCommandToHost("usbrelay HW554_5=1");
    if (!err.isEmpty())
    {
        emit boardLogReady(err);
        return;
    }

    emit boardLogReady("Waiting 10 seconds...");
    QElapsedTimer t;
    t.start();
    qint64 el = 0;
    do
    {
        el = t.elapsed();
        qApp->processEvents();
        QThread::usleep(100);
    }
    while (el < 10000);

    emit boardLogReady("Powering ON...");
    err = sendCommandToHost("usbrelay HW554_5=0");
    if (!err.isEmpty())
    {
        emit boardLogReady(err);
        return;
    }
    emit boardLogReady("Restart power cycle finished\n");
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

    /*
    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host) << QString("'%1%2'").arg(Settings.getScriptDir()).arg(Settings.AcquireScriptOnHost);
    qDebug() << "Starting acquisition:"<<command << args;
    */

    QStringList txt;
    txt << ". dabclogin\n";
    txt << QString("dabc_exe %1%2\n").arg(RunSettings.getScriptDir()).arg(RunSettings.StorageXML);

    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host);

    prAcquire = new QProcess();
    connect(prAcquire, SIGNAL(finished(int, QProcess::ExitStatus )), this, SLOT(onAcquireFinished(int,QProcess::ExitStatus)));
    connect(prAcquire, SIGNAL(readyRead()), this, SLOT(onReadyAcquireLog()));
    prAcquire->setProcessChannelMode(QProcess::MergedChannels);

    prAcquire->start(command, args);


    if (!prAcquire->waitForStarted(500))
    {
        delete prAcquire; prAcquire = nullptr;
        return "Could not start send";
    }

    for (const QString & s : txt)
        prAcquire->write(QByteArray(s.toLocal8Bit().data()));

    return "";
}

void ATrbRunControl::StopAcquire()
{
    QString command = "ssh";
    QStringList args;
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

    if (prAcquire)
    {
        prAcquire->closeWriteChannel();
    }
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
    emit sigAcquireOff();
}

void ATrbRunControl::recallConfiguration()
{
    emit boardLogReady("Board is connected");

    if (!Settings.isBufferRecordsEmpty())
    {
        const QString err = sendBufferControlToTRB();
        if (err.isEmpty()) emit boardLogReady("-->Updated buffer config");
        else emit boardLogReady("Error during sending buffer config:\n" + err);
    }
    const QString err = sendCTStoTRB();
    if (err.isEmpty()) emit boardLogReady("-->Updated trigger config");
       else emit boardLogReady("Error during sending trigger config:\n" + err);
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
        //if (log == "Starting CTS\n")
        if (log.contains("- trbnetd pid:"))
        {
            ConnectStatus = WaitingFirstReply;
            emit boardLogReady("Starting CTS");
        }
        emit boardLogReady(log);
        break;
    case WaitingFirstReply:
        if (log.contains("Register"))
        //if (log.contains("Label                               | Register"))
        {
            ConnectStatus = Connected;
            recallConfiguration(); // !
            emit sigBoardIsAlive(0);
        }
        else
        {
            emit boardLogReady(log);
        }
        break;
    case Connected:
        {
            //Triggers accepted                   | cts_cnt_trg_accepted       | 0xa002  |       923.18 |        88326
            int iStart = log.indexOf("cts_cnt_trg_accepted");
            double rate = 0;
            if (iStart > 0)
            {
                int iStop = log.indexOf('\n', iStart);
                QString str = log.mid(iStart, iStop-iStart);
                QStringList sl = str.split('|', QString::SkipEmptyParts);
                if (sl.size()>2)
                {
                    QString srate = sl.at(2).simplified();
                    rate = srate.toDouble();
                }
            }
            emit sigBoardIsAlive(rate); //"Expecting text every second - inicates the board is alive";
        }
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
                //events can be in kEvents (assume mEvents too)
                QString sEv = ff.at(1);
                if (sEv.right(1) == "k")
                {
                    sEv.chop(1);
                    StatEvents = sEv.toDouble() * 1000.0;
                }
                else if (sEv.right(1) == "m" || sEv.right(1) == "M")
                {
                    sEv.chop(1);
                    StatEvents = sEv.toDouble() * 1000000.0;
                }
                else StatEvents = sEv.toInt();

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

const QString ATrbRunControl::sendCommandToHost(const QString &command)
{
    QString com = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host);
    args << command.split(' ', QString::SkipEmptyParts);
    qDebug() << "Sending to host:" << com << args;

    QProcess pr;
    pr.setProcessChannelMode(QProcess::MergedChannels);
    pr.start(com, args);

    if (!pr.waitForStarted(500)) return "Could not start send";
    if (!pr.waitForFinished(2000)) return "Timeout on read reply";

    pr.closeWriteChannel();
    QString reply1 = pr.readAll();
    pr.close();
    qDebug() << reply1;
    return "";
}

#include <QXmlStreamReader>
#include <QFileInfo>
const QString ATrbRunControl::updateXML()
{
    QString where = sExchangeDir;
    if (!where.endsWith('/')) where += '/';

    QString err = sshCopyFileFromHost( QString("%1%2").arg(RunSettings.getScriptDir()).arg(RunSettings.StorageXML), where);
    if (!err.isEmpty()) return err;

    QString localFileName = where + RunSettings.StorageXML;
    QString hostDir = RunSettings.getScriptDir();

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
                    QString dir = RunSettings.HldDirOnHost;
                    if (!dir.endsWith('/')) dir += '/';
                    line = QString("<OutputPort name=\"Output1\" url=\"hld://%1dabc.hld?maxsize=%2\"/>\n").arg(dir).arg(RunSettings.MaxHldSizeMb);
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
    err = makeDirOnHost(RunSettings.HldDirOnHost);
    if (!err.isEmpty()) return "Failed to create target folder on host";

    return "";
}

#include "afiletools.h"
QString ATrbRunControl::updateCTSsetupScript()
{
    QString where = sExchangeDir;
    if (!where.endsWith('/')) where += '/';
    QString localCtsFileName = where + "TriggerSetup_Andr.sh";

    QStringList sl;
    sl << CtsSettingsToCommands(true);
    QString txt = sl.join("");
    qDebug() << txt;

    bool bOK = SaveTextToFile(localCtsFileName, txt);
    if (!bOK) return "Cannot save CTS settings to file " + localCtsFileName;

    QString hostDir = RunSettings.ScriptDirOnHost;
    qDebug() << "On host:" << hostDir << localCtsFileName;
    QString err = sshCopyFileToHost(localCtsFileName, hostDir);
    if (!err.isEmpty()) return err;

    return "";
}

QString ATrbRunControl::updateBufferSetupScript()
{
    if (Settings.getBufferRecords().isEmpty())
        return "Error: ADCs are not configured!";

    QString where = sExchangeDir;
    if (!where.endsWith('/')) where += '/';
    QString localCtsFileName = where + "BufferSetup_Andr.sh";

    const QStringList sl = bufferRecordsToCommands();
    QString commands = sl.join("");
    qDebug() << commands;

    bool bOK = SaveTextToFile(localCtsFileName, commands);
    if (!bOK) return "Cannot save buffer settings to file " + localCtsFileName;

    QString hostDir = RunSettings.ScriptDirOnHost;
    qDebug() << "On host:" << hostDir << localCtsFileName;
    QString err = sshCopyFileToHost(localCtsFileName, hostDir);
    if (!err.isEmpty()) return err;

    return "";
}

QString ATrbRunControl::sendCTStoTRB()
{
    QStringList txt;
    txt << CtsSettingsToCommands(false);
    qDebug() << txt;

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

QString ATrbRunControl::sendTriggerLogicToTRB()
{
    QStringList txt;
    txt << QString("trbcmd w 0xa003 0xdf00 0x%1 # OR logic, slice 0\n").arg( QString::number(Settings.TrbRunSettings.OR_0_FPGA3, 16) );
    txt << QString("trbcmd w 0xa003 0xdf04 0x%1 # OR logic, slice 1\n").arg( QString::number(Settings.TrbRunSettings.OR_1_FPGA3, 16) );
    txt << QString("trbcmd w 0xa004 0xdf00 0x%1 # OR logic, slice 0\n").arg( QString::number(Settings.TrbRunSettings.OR_0_FPGA4, 16) );
    txt << QString("trbcmd w 0xa004 0xdf04 0x%1 # OR logic, slice 1\n").arg( QString::number(Settings.TrbRunSettings.OR_1_FPGA4, 16) );
    qDebug() << txt;

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

const QStringList ATrbRunControl::bufferRecordsToCommands()
{
    QStringList txt;
    const QVector<ABufferRecord> & BufRec = Settings.getBufferRecords();
    for (const ABufferRecord & r : BufRec)
    {
        const QString addr = "0x" + QString::number(r.Datakind, 16);
        txt << QString("trbcmd w %1 0xa010 0x%2   #Buffer depth\n").arg(addr).arg(QString::number(r.Samples, 16));
        txt << QString("trbcmd w %1 0xa024 0x%2   #Buffer depth -> word count\n").arg(addr).arg(QString::number(r.Samples, 16));
        txt << QString("trbcmd w %1 0xa011 0x%2   #Samples after trigger\n").arg(addr).arg(QString::number(r.Delay, 16));
        txt << QString("trbcmd w %1 0xa015 0x%2   #Downsampling (starts from 0)\n").arg(addr).arg(QString::number(r.Downsampling, 16));
    }
    qDebug() << txt;
    return txt;
}

const QStringList ATrbRunControl::CtsSettingsToCommands(bool bIncludeHidden)
{
    QStringList txt;
    txt << "trbcmd setbit 0xc001 0xa00c 0x80000000   # -->Disable all triggers\n";  // master swicth off

    long bits = RunSettings.getTriggerInt();
    QString sbits = QString::number(bits, 16);
    txt << QString("trbcmd w 0xc001 0xa101 0x%1   # trg_channel and mask\n").arg(sbits);

    txt << QString("trbcmd w 0xc001 0xa15a %1   # random pulser frequency\n").arg(RunSettings.RandomPulserFrequency);
    txt << QString("trbcmd w 0xc001 0xa158 %1   # periodic pulser - period\n").arg(RunSettings.Period);

    txt << QString("trbcmd w 0xc001 0xa153 %1   # periphery trigger inpits 0\n").arg(RunSettings.PeripheryTriggerInputs0);
    txt << QString("trbcmd w 0xc001 0xa154 %1   # periphery trigger inputs 1\n").arg(RunSettings.PeripheryTriggerInputs1);

    if (bIncludeHidden)
    {
        txt << "#\n# The 'hidden' part:\n";
        txt << RunSettings.TheRestCTScontrols;
    }

    txt << "trbcmd clearbit 0xc001 0xa00c 0x80000000   # <--Enable all triggers\n"; // master switch on
    return txt;
}

QString ATrbRunControl::sendBufferControlToTRB()
{
    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host);

    qDebug() << "Sending command/args:" << command << args;

    QProcess pr;
    pr.setProcessChannelMode(QProcess::MergedChannels);
    pr.start(command, args);

    if (!pr.waitForStarted(500)) return "Could not start send";

    const QStringList txt = bufferRecordsToCommands();
    for (const QString & s : txt)
        pr.write(QByteArray(s.toLocal8Bit().data()));

    pr.closeWriteChannel();
    if (!pr.waitForFinished(2000)) return "Timeout on attempt to execute Buffer configuration";

    QString reply1 = pr.readAll();

    pr.close();

    qDebug() << reply1;
    return "";
}

QString ATrbRunControl::readBufferControlFromTRB()
{
    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host);

    qDebug() << "Sending command/args:" << command << args;

    QProcess pr;
    pr.setProcessChannelMode(QProcess::MergedChannels);
    pr.start(command, args);

    if (!pr.waitForStarted(500)) return "Could not start send";

    QStringList txt;
    const QVector<ABufferRecord> & BufRecConst = Settings.getBufferRecords();
    for (const ABufferRecord & r : BufRecConst)
    {
        const QString addr = "0x" + QString::number(r.Datakind, 16);

        txt << QString("trbcmd r %1 0xa010\n").arg(addr);
        txt << QString("trbcmd r %1 0xa011\n").arg(addr);
        txt << QString("trbcmd r %1 0xa015\n").arg(addr);
    }
    for (const QString & s : txt) pr.write(QByteArray(s.toLocal8Bit().data()));

    pr.closeWriteChannel();
    if (!pr.waitForFinished(2000)) return "Timeout on attempt to execute Buffer configuration";

    QString reply = pr.readAll();
    QStringList sl = reply.split('\n', QString::SkipEmptyParts);

    //clean login messages
    while (!sl.isEmpty() && !sl.first().startsWith("0x"))
        sl.removeFirst();
    qDebug() << sl;

    if (sl.size() != BufRecConst.size() * 3) // absolute number = number of settings ber buf!
        return "unexpected number of reply lines";

    QVector<ABufferRecord> & BufRec = Settings.getBufferRecords();
    int icounter = 0;
    for (ABufferRecord & r : BufRec)
    {
        const QString addr = "0x" + QString::number(r.Datakind, 16);
        ulong Samples, Delay, Downsampling;
        bool bOK;

        QStringList l = sl.at(icounter).split(' ', QString::SkipEmptyParts);
        if (l.size() !=2 || l.first() != addr)
            return "unexpected format of reply line";
        Samples = l.last().toULong(&bOK, 16);
        if (!bOK) return "unexpected format of reply line";
        icounter++;

        l = sl.at(icounter).split(' ', QString::SkipEmptyParts);
        if (l.size() !=2 || l.first() != addr)
            return "unexpected format of reply line";
        Delay = l.last().toULong(&bOK, 16);
        if (!bOK) return "unexpected format of reply line";
        icounter++;

        l = sl.at(icounter).split(' ', QString::SkipEmptyParts);
        if (l.size() !=2 || l.first() != addr)
            return "unexpected format of reply line";
        Downsampling = l.last().toULong(&bOK, 16);
        if (!bOK) return "unexpected format of reply line";
        icounter++;

        r.Samples = Samples;
        r.Delay = Delay;
        r.Downsampling = Downsampling;
    }

    pr.close();
    return "";
}

QString ATrbRunControl::sendTimeSettingsToTRB()
{
    QStringList txt;
    txt << QString("trbcmd w 0xa003 0xc802 0x%1 # activated time channels\n").arg( QString::number(Settings.TrbRunSettings.TimeChannels_FPGA3, 16) );
    unsigned val = 0x80000000 + Settings.TrbRunSettings.TimeWinBefore_FPGA3/5*0x10000 + Settings.TrbRunSettings.TimeWinAfter_FPGA3/5;
    qDebug() << "0x"+QString::number(val, 16);
    txt << QString("trbcmd w 0xa003 0xc801 0x%1 # before/after windows\n").arg( QString::number(val, 16) );
    txt << QString("trbcmd w 0xa003 0xc804 0x0000007b # set max data limit\n");

    txt << QString("trbcmd w 0xa004 0xc802 0x%1 # activated time channels\n").arg( QString::number(Settings.TrbRunSettings.TimeChannels_FPGA4, 16) );
    val = 0x80000000 + Settings.TrbRunSettings.TimeWinBefore_FPGA4/5*0x10000 + Settings.TrbRunSettings.TimeWinAfter_FPGA4/5;
    qDebug() << "0x"+QString::number(val, 16);
    txt << QString("trbcmd w 0xa004 0xc801 0x%1 # before/after windows\n").arg( QString::number(val, 16) );
    txt << QString("trbcmd w 0xa004 0xc804 0x0000007b # set max data limit\n");

    qDebug() << txt;

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

QString ATrbRunControl::readTimeSettingsFromTRB()
{
    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host);

    qDebug() << "Sending command/args:" << command << args;

    QProcess pr;
    pr.setProcessChannelMode(QProcess::MergedChannels);
    pr.start(command, args);

    if (!pr.waitForStarted(500)) return "Could not start send";

    QStringList txt;
    const QVector<QString> TimeBoards = {"0xa003", "0xa004"};
    for (const QString & bstr : TimeBoards)
    {
        txt << QString("trbcmd r %1 0xc802\n").arg(bstr);
        txt << QString("trbcmd r %1 0xc801\n").arg(bstr);
    }
    for (const QString & s : txt) pr.write(QByteArray(s.toLocal8Bit().data()));

    pr.closeWriteChannel();
    if (!pr.waitForFinished(2000)) return "Timeout on attempt to execute Buffer configuration";

    QString reply = pr.readAll();
    QStringList sl = reply.split('\n', Qt::SkipEmptyParts);

    //clean login messages
    while (!sl.isEmpty() && !sl.first().startsWith("0x"))
        sl.removeFirst();
    qDebug() << sl;

    if (sl.size() != TimeBoards.size() * 2) // absolute number = number of settings ber buf!
        return "unexpected number of reply lines";

    bool bOK;

    QStringList l = sl[0].split(' ', Qt::SkipEmptyParts);
    if (l.size() !=2 || l.first() != TimeBoards[0]) return "unexpected format of reply line";
    Settings.TrbRunSettings.TimeChannels_FPGA3 = l.last().toULong(&bOK, 16);
    if (!bOK) return "unexpected format of reply line";

    l = sl[1].split(' ', Qt::SkipEmptyParts);
    if (l.size() !=2 || l.first() != TimeBoards[0]) return "unexpected format of reply line";
    unsigned compoundVal = l.last().toULong(&bOK, 16);
    if (!bOK) return "unexpected format of reply line";
    Settings.TrbRunSettings.TimeWinBefore_FPGA3 = ((compoundVal/0x10000) & 0x7fff) * 5;
    Settings.TrbRunSettings.TimeWinAfter_FPGA3 = (compoundVal & 0x7fff) * 5;

    l = sl[2].split(' ', Qt::SkipEmptyParts);
    if (l.size() !=2 || l.first() != TimeBoards[1]) return "unexpected format of reply line";
    Settings.TrbRunSettings.TimeChannels_FPGA4 = l.last().toULong(&bOK, 16);
    if (!bOK) return "unexpected format of reply line";

    l = sl[3].split(' ', Qt::SkipEmptyParts);
    if (l.size() !=2 || l.first() != TimeBoards[1]) return "unexpected format of reply line";
    compoundVal = l.last().toULong(&bOK, 16);
    if (!bOK) return "unexpected format of reply line";
    Settings.TrbRunSettings.TimeWinBefore_FPGA4 = ((compoundVal/0x10000) & 0x7fff) * 5;
    Settings.TrbRunSettings.TimeWinAfter_FPGA4 = (compoundVal & 0x7fff) * 5;

    pr.close();
    return "";
}

QString ATrbRunControl::readTriggerLogicFromTRB()
{
    QString command = "ssh";
    QStringList args;
    args << QString("%1@%2").arg(User).arg(Host);

    qDebug() << "Sending command/args:" << command << args;

    QProcess pr;
    pr.setProcessChannelMode(QProcess::MergedChannels);
    pr.start(command, args);

    if (!pr.waitForStarted(500)) return "Could not start send";

    QStringList txt;
    const QVector<QString> TimeBoards = {"0xa003", "0xa004"};
    for (const QString & bstr : TimeBoards)
    {
        txt << QString("trbcmd r %1 0xdf00\n").arg(bstr);
        txt << QString("trbcmd r %1 0xdf04\n").arg(bstr);
    }
    for (const QString & s : txt) pr.write(QByteArray(s.toLocal8Bit().data()));

    pr.closeWriteChannel();
    if (!pr.waitForFinished(2000)) return "Timeout on attempt to execute Buffer configuration";

    QString reply = pr.readAll();
    QStringList sl = reply.split('\n', Qt::SkipEmptyParts);

    //clean login messages
    while (!sl.isEmpty() && !sl.first().startsWith("0x"))
        sl.removeFirst();
    qDebug() << sl;

    if (sl.size() != TimeBoards.size() * 2) // absolute number = number of settings ber buf!
        return "unexpected number of reply lines";

    bool bOK;

    QStringList l = sl[0].split(' ', Qt::SkipEmptyParts);
    if (l.size() !=2 || l.first() != TimeBoards[0]) return "unexpected format of reply line";
    Settings.TrbRunSettings.OR_0_FPGA3 = l.last().toULong(&bOK, 16);
    if (!bOK) return "unexpected format of reply line";

    l = sl[1].split(' ', Qt::SkipEmptyParts);
    if (l.size() !=2 || l.first() != TimeBoards[0]) return "unexpected format of reply line";
    Settings.TrbRunSettings.OR_1_FPGA3 = l.last().toULong(&bOK, 16);
    if (!bOK) return "unexpected format of reply line";

    l = sl[2].split(' ', Qt::SkipEmptyParts);
    if (l.size() !=2 || l.first() != TimeBoards[1]) return "unexpected format of reply line";
    Settings.TrbRunSettings.OR_0_FPGA4 = l.last().toULong(&bOK, 16);
    if (!bOK) return "unexpected format of reply line";

    l = sl[3].split(' ', Qt::SkipEmptyParts);
    if (l.size() !=2 || l.first() != TimeBoards[1]) return "unexpected format of reply line";
    Settings.TrbRunSettings.OR_1_FPGA4 = l.last().toULong(&bOK, 16);
    if (!bOK) return "unexpected format of reply line";

    pr.close();
    return "";
}

void ATrbRunControl::checkFreeSpace()
{
    if (prFreeSpaceChecker)
    {
        //nothing to do - wait for finished
    }
    else
    {
        QString dir = RunSettings.HldDirOnHost;
        if (dir.isEmpty()) return;

        QString command = "ssh";
        QStringList args;

        //QStringList sl = dir.split('/',QString::SkipEmptyParts);
        //if (sl.isEmpty()) dir = "/";
        //else dir = '/' + sl.first();

        args << QString("%1@%2").arg(User).arg(Host) << "df" << dir;

        prFreeSpaceChecker = new QProcess();
        connect(prFreeSpaceChecker, SIGNAL(finished(int, QProcess::ExitStatus )), this, SLOT(onFreeSpaceCheckerFinished()));
        connect(prFreeSpaceChecker, SIGNAL(readyRead()), this, SLOT(onFreeSpaceCheckerReady()));
        prFreeSpaceChecker->setProcessChannelMode(QProcess::MergedChannels);

        lastFreeSpace = -1;
        timerFreeSpaceChecker->start(2000);
        prFreeSpaceChecker->start(command, args);
    }
}

void ATrbRunControl::onFreeSpaceCheckerFinished()
{
    timerFreeSpaceChecker->stop();//paranoic

    prFreeSpaceChecker->closeWriteChannel();
    prFreeSpaceChecker->closeReadChannel(QProcess::StandardOutput);
    prFreeSpaceChecker->closeReadChannel(QProcess::StandardError);

    prFreeSpaceChecker->deleteLater();
    prFreeSpaceChecker = nullptr;
}

void ATrbRunControl::onFreeSpaceCheckerReady()
{
    timerFreeSpaceChecker->stop();
    lastFreeSpace = -1;

    QString log(prFreeSpaceChecker->readAll());

    if (log.startsWith("Filesystem"))
    {
        QStringList sl = log.split('\n');
        if (sl.size() > 1)
        {
            QString line = sl.first();
            QStringList f = line.split(' ', QString::SkipEmptyParts);
            if (f.size() > 2 )
            {
                int blockSize = 1024; //default
                QString br = f.at(1);
                f = br.split('-', QString::SkipEmptyParts);
                QString record = f.first();
                bool bOK;
                if (record.endsWith('k') || record.endsWith('K'))
                {
                    //qDebug() << "in K bytes";
                    record.chop(1);
                    int iTest = record.toInt(&bOK) ;
                    if (bOK)
                    {
                        blockSize = 1024 * iTest;
                        //qDebug() << "Block size is "<< blockSize << "bytes";
                    }
                    else qDebug() << "Size convertion to int failed, assuimg 1K blocks";
                }
                else
                {
                    int iTest = record.toInt(&bOK);
                    if (bOK)
                    {
                        blockSize = iTest;
                        //qDebug() << "Block size is "<< blockSize << "bytes";
                    }
                    else qDebug() << "Size convertion to int failed, assuimg 1K blocks";
                }

                //in blocks
                line = sl.at(1);
                f = line.split(' ', QString::SkipEmptyParts);
                if (f.size() > 4 )
                {
                    QString ssize = f.at(3);
                    bool bOK;
                    long isize = ssize.toInt(&bOK);
                    if (bOK)
                        lastFreeSpace = isize * blockSize;
                }
            }
        }
    }
    //qDebug() << lastFreeSpace;
    prFreeSpaceChecker->closeWriteChannel();
    emit freeSpaceCheckReady(lastFreeSpace);
}

void ATrbRunControl::onFreeSpaceCheckerTimeout()
{
    if (prFreeSpaceChecker)
    {
        delete prFreeSpaceChecker; prFreeSpaceChecker = nullptr;
        lastFreeSpace = -1;
        emit freeSpaceCheckReady(-1);
    }
}

QString ATrbRunControl::ReadTriggerSettingsFromBoard()
{
    //http://192.168.3.214:1234/cts/cts.pl?dump,shell

    QString reply;
    QString url = QString("http://%1:1234/cts/cts.pl?dump,shell").arg(RunSettings.Host);
    bool bOK = Network.makeHttpRequest(url, reply, 2000);

    if (!bOK) return reply;

    if (reply.startsWith("# CTS Configuration dump") && reply.endsWith("# Enable all triggers\n"))
    {
        RunSettings.TheRestCTScontrols.clear();
        const QStringList sl = reply.split('\n', QString::SkipEmptyParts);
        for (const QString & s : sl)
        {
            QStringList line = s.split(' ', QString::SkipEmptyParts);
            if (line.size() < 5) continue;

            if (line.at(3) == "0xa101")
            {
                QString trig = line.at(4);
                RunSettings.setTriggerInt( trig.toULong(nullptr, 16) );
            }
            else if (line.at(3) == "0xa15a") RunSettings.RandomPulserFrequency = line.at(4);
            else if (line.at(3) == "0xa158") RunSettings.Period = line.at(4);
            else if (line.at(3) == "0xa153") RunSettings.PeripheryTriggerInputs0 = line.at(4);
            else if (line.at(3) == "0xa154") RunSettings.PeripheryTriggerInputs1 = line.at(4);
            else
            {
                if (!s.contains("setbit") && !s.contains("clearbit"))
                    RunSettings.TheRestCTScontrols << s + '\n';
            }
        }
        return "";
    }
    return "Error: Board returned string with wrong format";
}
