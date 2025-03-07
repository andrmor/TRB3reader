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

    //bool ProcessFile(const QString FileName, bool bSaveTimeData, const QString SaveFileName = "", bool doNotSaveSuppressedChannels = false);
    bool ProcessFile(const QString FileName, int What_0signals1waves, bool bIncludeTimeData, const QString SaveFileName, bool doNotSaveSuppressedChannels);
    bool SaveSignalsToFile(const QString & FileName, bool bUseHardware, bool bSaveTimeData, bool doNotSaveSuppressed);
    bool SaveWaveformsToFile(const QString & FileName, bool bUseHardware, bool bSaveTimeData, bool doNotSaveSuppressed);

    const QString& GetLastError() const {return LastError;}

private:
    const MasterConfig  & Config;
    Trb3dataReader      & Reader;
    Trb3signalExtractor & Extractor;

    QString LastError;

private:
    bool sendSignalData(QTextStream &outStream, bool bUseHardware, bool bSaveTimeData, bool doNotSaveSuppressed);
    bool sendWaveformData(QTextStream &outStream, bool bUseHardware, bool bSaveTimeData, bool doNotSaveSuppressed);

    void saveTimeData(int iEvent, QTextStream &outStream);
signals:
    void LogAction(const QString currentAction);
    void LogMessage(const QString message);
    void RequestExecuteScript(bool& returnState);
};

#endif // AHLDFILEPROCESSOR_H
