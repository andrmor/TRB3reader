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

    bool ProcessFile(const QString& FileName);
    bool SaveSignalsToFile(const QString& FileName, bool bUseHardware);

    const QString& GetLastError() const {return LastError;}


private:
    MasterConfig& Config;
    Trb3dataReader& Reader;
    Trb3signalExtractor& Extractor;
    ADataHub& DataHub;
    QString LastError;

private:
    bool sendSignalData(QTextStream &outStream, bool bUseHardware);

signals:
    void LogAction(const QString currentAction);
    void LogMessage(const QString message);
    void RequestExecuteScript(bool& returnState);
};

#endif // AHLDFILEPROCESSOR_H
