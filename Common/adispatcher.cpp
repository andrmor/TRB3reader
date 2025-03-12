#include "adispatcher.h"
#include "masterconfig.h"
#include "channelmapper.h"
#include "trb3datareader.h"
#include "trb3signalextractor.h"
#include "ajsontools.h"
#include "anetworkmodule.h"

#include "mainwindow.h"
//#include "ascriptwindow.h"

#include <QDebug>
#include <QDir>
#include <QJsonObject>

ADispatcher::ADispatcher(Trb3dataReader * Reader, Trb3signalExtractor* Extractor, ANetworkModule* Network) :
    Config(MasterConfig::getInstance()),Reader(Reader), Extractor(Extractor), Network(Network) {}

void ADispatcher::LoadAutosaveConfig()
{
    if (!QDir(Config.ConfigDir).exists()) QDir().mkdir(Config.ConfigDir);
    else LoadConfig(Config.AutosaveFile);
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
    LoadConfig(json, true);
}

bool ADispatcher::LoadConfig(QJsonObject & json, bool includeGui)
{
    Config.ReadFromJson(json);
    ClearData();

    if (includeGui)
    {
        emit RequestReadGuiFromJson(json);
        emit RequestUpdateGui();
    }

    return true;
}

void ADispatcher::SaveConfig(const QString FileName)
{
    QJsonObject json;
    Config.WriteToJson(json);

    emit RequestWriteGuiToJson(json);
    SaveJsonToFile(json, FileName);

    emit RequestWriteWindowSettings();
}

void ADispatcher::ClearNegativeChannels()
{
    Config.SetNegativeChannels(QVector<int>());
    ClearData();

    emit RequestUpdateGui();
}

void ADispatcher::ClearMapping()
{
    Config.SetMapping(QVector<int>());
    ClearData();

    emit RequestUpdateGui();
}

void ADispatcher::ClearIgnoreChannels()
{
    Config.ClearListOfIgnoreChannels();
    ClearData();

    emit RequestUpdateGui();
}

