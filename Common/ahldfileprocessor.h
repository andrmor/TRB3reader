#ifndef AHLDFILEPROCESSOR_H
#define AHLDFILEPROCESSOR_H

#include <QObject>
#include <QString>

class MasterConfig;
class Trb3dataReader;
class Trb3signalExtractor;
class QTextStream;

class AHldFileProcessor : public QObject
{
    Q_OBJECT

public:
    AHldFileProcessor(Trb3dataReader & Reader, Trb3signalExtractor & Extractor);

    bool processFile(const QString & FileName, int What_0signals1waves, bool bIncludeTimeData, const QString & SaveFileName, bool doNotSaveSuppressedChannels, bool doLogs);

    QString LastError;

private:
    const MasterConfig  & Config;
    Trb3dataReader      & Reader;
    Trb3signalExtractor & Extractor;

private:
    bool SaveSignalsToFile(const QString & FileName, bool bUseHardware, bool bSaveTimeData, bool doNotSaveSuppressed);
    bool SaveWaveformsToFile(const QString & FileName, bool bUseHardware, bool bSaveTimeData, bool doNotSaveSuppressed);
    bool sendSignalData(QTextStream &outStream, bool bUseHardware, bool bSaveTimeData, bool doNotSaveSuppressed);
    bool sendWaveformData(QTextStream &outStream, bool bUseHardware, bool bSaveTimeData, bool doNotSaveSuppressed);

    void saveTimeData(int iEvent, QTextStream &outStream);
signals:
    void LogAction(const QString currentAction);
    void LogMessage(const QString message);
};

#endif // AHLDFILEPROCESSOR_H
