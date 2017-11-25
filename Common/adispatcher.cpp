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

//ADispatcher::ADispatcher(MasterConfig* Config, ChannelMapper* Map, Trb3dataReader* Reader, Trb3signalExtractor* Extractor) :
//    Config(Config), Map(Map), Reader(Reader), Extractor(Extractor)
//{
//    onStart();
//}

ADispatcher::ADispatcher(MasterConfig* Config, Trb3dataReader* Reader, Trb3signalExtractor* Extractor, MainWindow *MW) :
    Config(Config),Reader(Reader), Extractor(Extractor), MW(MW)
{
    onStart();
}

void ADispatcher::onStart()
{
    //finding the config dir
    ConfigDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/TRBreader";
    AutosaveFile = ConfigDir+"/autosave.json";
    if (!QDir(ConfigDir).exists())
        QDir().mkdir(ConfigDir);
    else
        LoadConfig(AutosaveFile);

    qDebug() << "-> Config dir:" << ConfigDir;
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

    MW->readGUIfromJson(json);
    MW->UpdateGui();

    return true;
}

void ADispatcher::SaveConfig(QString FileName, QJsonObject js)
{
    QJsonObject json;
    Config->WriteToJson(json);

    MW->writeGUItoJson(json);
    MW->writeWindowsToJson(json, js);

    SaveJsonToFile(json, FileName);
}

void ADispatcher::ClearNegativeChannels()
{
    Config->SetNegativeChannels(std::vector<int>());
    ClearData();

    MW->UpdateGui();
}

void ADispatcher::ClearMapping()
{
    Config->SetMapping(QVector<int>());
    ClearData();

    MW->UpdateGui();
}

void ADispatcher::ClearIgnoreChannels()
{
    Config->IgnoreHardwareChannels.clear();
    ClearData();

    MW->UpdateGui();
}
