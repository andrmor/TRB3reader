#include "adispatcher.h"
#include "masterconfig.h"
#include "channelmapper.h"
#include "trb3datareader.h"
#include "trb3signalextractor.h"
#include "ajsontools.h"
#include "anetworkmodule.h"

#include "mainwindow.h"
#include "ascriptwindow.h"

#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QJsonObject>

ADispatcher::ADispatcher(MasterConfig* Config, Trb3dataReader* Reader, Trb3signalExtractor* Extractor, ANetworkModule* Network) :
    Config(Config),Reader(Reader), Extractor(Extractor), Network(Network)
{
    ConfigDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/TRBreader";
    AutosaveFile = ConfigDir+"/autosave.json";
    WinSetFile = ConfigDir+"/winset.json";
    qDebug() << "-> Config dir:" << ConfigDir;
}

void ADispatcher::LoadAutosaveConfig()
{
    if (!QDir(ConfigDir).exists()) QDir().mkdir(ConfigDir);
    else LoadConfig(AutosaveFile);
}

void ADispatcher::ClearData()
{
    Reader->ClearData();
    Extractor->ClearData();
}

void ADispatcher::LoadConfig(const QString FileName)
{
    QJsonObject json;
    LoadJsonFromFile(json, FileName);
    LoadConfig(json);
}

bool ADispatcher::LoadConfig(QJsonObject &json)
{
    Config->ReadFromJson(json);
    ClearData();

    emit RequestReadGuiFromJson(json);
    emit RequestUpdateGui();

    return true;
}

void ADispatcher::SaveConfig(const QString FileName)
{
    QJsonObject json;
    Config->WriteToJson(json);

    emit RequestWriteGuiToJson(json);
    SaveJsonToFile(json, FileName);

    emit RequestWriteWindowSettings();
}

void ADispatcher::ClearNegativeChannels()
{
    Config->SetNegativeChannels(QVector<int>());
    ClearData();

    emit RequestUpdateGui();
}

void ADispatcher::ClearMapping()
{
    Config->SetMapping(QVector<int>());
    ClearData();

    emit RequestUpdateGui();
}

void ADispatcher::ClearIgnoreChannels()
{
    Config->ClearListOfIgnoreChannels();
    ClearData();

    emit RequestUpdateGui();
}

const QString ADispatcher::ReadTriggerSettingsFromBoard()
{
    //http://192.168.3.214:1234/cts/cts.pl?dump,shell

    QString reply;
    QString url = QString("http://%1:1234/cts/cts.pl?dump,shell").arg(Config->TrbRunSettings.Host);
    bool bOK = Network->makeHttpRequest(url, reply, 2000);

    if (!bOK) return reply;

    if (reply.startsWith("# CTS Configuration dump") && reply.endsWith("# Enable all triggers\n"))
    {
        QString txt = reply;
        return "";
    }
    return "Error: Board returned string with wrong format";
}
