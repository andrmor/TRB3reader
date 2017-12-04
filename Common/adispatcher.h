#ifndef ADISPATCHER_H
#define ADISPATCHER_H

#include <QObject>
#include <QString>

class MasterConfig;
class Trb3dataReader;
class Trb3signalExtractor;
class QJsonObject;

class MainWindow;

class QJsonObject;

class ADispatcher : public QObject
{
    Q_OBJECT

public:
    ADispatcher(MasterConfig* Config, Trb3dataReader* Reader, Trb3signalExtractor* Extractor);

    void ClearData();

    void LoadAutosaveConfig();
    void LoadConfig(const QString FileName);
    bool LoadConfig(QJsonObject& json);
    void SaveConfig(const QString FileName);

    void ClearNegativeChannels();
    void ClearMapping();
    void ClearIgnoreChannels();

public:
    QString ConfigDir;
    QString AutosaveFile;
    QString WinSetFile;

private:
    MasterConfig* Config;
    Trb3dataReader* Reader;
    Trb3signalExtractor* Extractor;

    //interaction with GUI through the signals/slots
signals:
    void RequestUpdateGui();
    void RequestReadGuiFromJson(const QJsonObject& json);
    void RequestWriteGuiToJson(QJsonObject& json);
    void RequestWriteWindowSettings();
};

#endif // ADISPATCHER_H
