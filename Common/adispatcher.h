#ifndef ADISPATCHER_H
#define ADISPATCHER_H

#include <QObject>
#include <QString>

class MasterConfig;
class ChannelMapper;
class Trb3dataReader;
class Trb3signalExtractor;

class MainWindow;

class QJsonObject;

class ADispatcher
{
public:
    //ADispatcher(MasterConfig* Config, ChannelMapper* Map, Trb3dataReader* Reader, Trb3signalExtractor* Extractor); //No GUI
    ADispatcher(MasterConfig* Config, ChannelMapper* Map, Trb3dataReader* Reader, Trb3signalExtractor* Extractor, MainWindow* MW); //With GUI

    void ClearData();

    void LoadConfig(QString FileName);
    bool LoadConfig(QJsonObject& json);
    void SaveConfig(QString FileName, QJsonObject js);

    void ClearNegativeChannels();
    void ClearMapping();
    void ClearIgnoreChannels();

public:
    QString ConfigDir;
    QString AutosaveFile;


private:
    MasterConfig* Config;
    ChannelMapper* Map;
    Trb3dataReader* Reader;
    Trb3signalExtractor* Extractor;

    //GUI
    MainWindow* MW;

    void onStart();
};

#endif // ADISPATCHER_H
