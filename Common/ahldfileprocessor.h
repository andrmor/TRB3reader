#ifndef AHLDFILEPROCESSOR_H
#define AHLDFILEPROCESSOR_H

#include <QObject>
#include <QString>

class MasterConfig;
class Trb3dataReader;
class Trb3signalExtractor;
class ADataHub;
class QTextStream;

class AHldFileProcessor : public QObject
{
    Q_OBJECT

public:
    AHldFileProcessor(MasterConfig& Config,
                      Trb3dataReader& Reader,
                      Trb3signalExtractor& Extractor,
                      ADataHub& DataHub);

    bool ProcessFile(const QString FileName, bool bSaveTimeData, const QString SaveFileName = "");
    bool SaveSignalsToFile(const QString FileName, bool bUseHardware, bool bSaveTimeData);

    const QString& GetLastError() const {return LastError;}


private:
    MasterConfig& Config;
    Trb3dataReader& Reader;
    Trb3signalExtractor& Extractor;
    ADataHub& DataHub;
    QString LastError;

private:
    bool sendSignalData(QTextStream &outStream, bool bUseHardware, bool bSaveTimeData);  // remove commenting inside to increase data precision!

    void saveTimeData(int iEvent, QTextStream &outStream);
signals:
    void LogAction(const QString currentAction);
    void LogMessage(const QString message);
    void RequestExecuteScript(bool& returnState);
};

#endif // AHLDFILEPROCESSOR_H
