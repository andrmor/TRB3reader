#include "adispatcher.h"
#include "masterconfig.h"
#include "channelmapper.h"
#include "trb3datareader.h"
#include "trb3signalextractor.h"
#include "ajsontools.h"

#include "mainwindow.h"
#include "ascriptwindow.h"

#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QJsonObject>

ADispatcher::ADispatcher(MasterConfig* Config, Trb3dataReader* Reader, Trb3signalExtractor* Extractor) :
    Config(Config),Reader(Reader), Extractor(Extractor)
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

void ADispatcher::LoadConfig(QString FileName)
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

void ADispatcher::SaveConfig(QString FileName)
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
