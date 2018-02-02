#include "ahldfileprocessor.h"
#include "masterconfig.h"
#include "trb3datareader.h"
#include "trb3signalextractor.h"
#include "adatahub.h"
#include "channelmapper.h"

#include <QFileInfo>
#include <QDebug>

AHldFileProcessor::AHldFileProcessor(MasterConfig& Config,
                                     Trb3dataReader& Reader,
                                     Trb3signalExtractor& Extractor,
                                     ADataHub& DataHub) :
    Config(Config), Reader(Reader), Extractor(Extractor), DataHub(DataHub) {}

bool AHldFileProcessor::ProcessFile(const QString FileName, const QString SaveFileName)
{
    if (FileName.isEmpty())
    {
        LogMessage("File name is empty!");
        LastError = "File name is not defined";
        return false;
    }
    emit LogAction("Reading file...");
    emit LogMessage("Processing " + QFileInfo(FileName).fileName());
    qDebug() << "Processing" <<  FileName;

    // Reading waveforms, pefroming optional smoothing/pedestal substraction
    bool ok = Reader.Read(FileName);
    if (!ok)
    {
        LogMessage("Read failed or all events were rejected!");
        LastError = "Read failed or all events were rejected";
        return false;
    }
    const QString ValRes = Config.Map->ValidateForAvailableHardwareChannels(Reader.CountChannels());
    if (!ValRes.isEmpty())
    {
        LogMessage(ValRes);
        return false;
    }

    //numProcessedEvents += Reader.CountAllProcessedEvents();
    //numBadEvents += Reader.CountBadEvents();

    // Extracting signals (or generating dummy data if disabled)
    Extractor.ClearData();
    if (Config.HldProcessSettings.bDoSignalExtraction)
    {
        LogAction("Extracting signals...");
        bool bOK = Extractor.ExtractSignals();
        if (!bOK)
        {
            LogMessage("Signal extraction failed!");
            LastError = "Signal extraction failed";
            return false;
        }
    }
    else
    {
        LogAction("Generating default 0 signals");
        qDebug() << "Generating default data (all signals = 0) in extractor data";
        Extractor.GenerateDummyData();
    }

    // Executing script
    if (Config.HldProcessSettings.bDoScript)
    {
        LogAction("Executing script...");
        bool bOK;
        emit RequestExecuteScript(bOK);
        if (!bOK)
        {
            LogMessage("Script execution error");
            LastError = "Script execution error";
            return false;
        }
    }

    // Checking that after extraction/script the data are consistent in num channels / mapping
    int numEvents = Extractor.CountEvents();
    int numChannels = Extractor.CountChannels();
    if (numEvents == 0 || numChannels == 0)
    {
        emit LogMessage("Extractor data not valid -> ignoring this file");
        LastError = "Extractor data not valid -> ignoring this file";
        return false;
    }

    // saving processed data to file
    if (Config.HldProcessSettings.bDoSave || !SaveFileName.isEmpty())
    {
        QString nameSave;
        if (SaveFileName.isEmpty())
        {
            QFileInfo fi(FileName);
            nameSave = fi.path() + "/" + fi.completeBaseName() + Config.HldProcessSettings.AddToFileName;
        }
        else nameSave = SaveFileName;

        qDebug() << "Saving to file:"<< nameSave;
        LogAction("Saving to file...");
        bool bOK = SaveSignalsToFile(nameSave, false);
        if (!bOK) return false;
    }

    //Coping data to DataHub
    if (Config.HldProcessSettings.bDoCopyToDatahub)
    {
        LogAction("Copying to datahub...");
        qDebug() << "Copying to DataHub...";
        const QVector<int>& map = Config.Map->GetMapToHardware();

        for (int iev=0; iev<numEvents; iev++)
        {
            if (Extractor.IsRejectedEventFast(iev)) continue;

            AOneEvent* ev = new AOneEvent;

            //signals
            const QVector<float>* vecHardw = Extractor.GetSignalsFast(iev);
            QVector<float> vecLogical;
            for (int ihardw : map) vecLogical << vecHardw->at(ihardw);
            ev->SetSignals(&vecLogical);

            //rejection status
            ev->SetRejectedFlag(false);

            //waveforms
            if (Config.HldProcessSettings.bCopyWaveforms)
            {
                QVector< QVector<float>* > vec;
                for (int ihardw : map)
                {
                    if (Extractor.GetSignalFast(iev, ihardw) == 0 || Config.IsIgnoredHardwareChannel(ihardw)) vec << 0;
                    else
                    {
                        QVector<float>* wave = new QVector<float>();
                        *wave = *Reader.GetWaveformPtrFast(iev, ihardw);
                        vec << wave;
                    }
                }
                ev->SetWaveforms(&vec);
            }

            DataHub.AddEventFast(ev);
        }
    }

    return true;
}

bool AHldFileProcessor::SaveSignalsToFile(const QString FileName, bool bUseHardware)
{
    QFile outputFile(FileName);
    outputFile.open(QIODevice::WriteOnly);
    if(!outputFile.isOpen())
        {
          //QMessageBox::warning(this, "TRB3reader", "Unable to open file!", QMessageBox::Ok, QMessageBox::Ok);
          emit LogMessage("Failed to save signals");
          LastError = "Unable to open file to save signals: " + FileName;
          return false;
        }

    QTextStream outStream(&outputFile);
    sendSignalData(outStream, bUseHardware);
    if (bUseHardware) LogAction("Signals saved using HARDWARE channels!");
    else LogAction("Signals saved");
    outputFile.close();
    return true;
}

bool AHldFileProcessor::sendSignalData(QTextStream &outStream, bool bUseHardware)
{
    int numEvents = Extractor.CountEvents();
    int numChannels = Extractor.CountChannels();

    if (bUseHardware)
    {
        for (int ie=0; ie<numEvents; ie++)
            if (!Extractor.IsRejectedEventFast(ie))
            {
                for (int ic=0; ic<numChannels; ic++)
                    outStream << Extractor.GetSignalFast(ie, ic) << " ";
                outStream << "\r\n";
            }
    }
    else
    {
        numChannels = Config.Map->CountLogicalChannels();
        for (int ie=0; ie<numEvents; ie++)
            if (!Extractor.IsRejectedEventFast(ie))
            {
                for (int ic=0; ic<numChannels; ic++)
                    outStream << Extractor.GetSignalFast(ie, Config.Map->LogicalToHardwareFast(ic)) << " ";
                outStream << "\r\n";
            }
    }
    return true;
}
