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

bool AHldFileProcessor::ProcessFile(const QString FileName, bool bSaveTimeData, const QString SaveFileName)
{
    if (FileName.isEmpty())
    {
        emit LogMessage("File name is empty!");
        LastError = "File name is not defined";
        return false;
    }
    emit LogAction("Reading file...");
    emit LogMessage("Processing " + QFileInfo(FileName).fileName());
    qDebug() << "Processing" <<  FileName;

    // Reading waveforms, pefroming optional smoothing/pedestal substraction
    LastError = Reader.Read(FileName);
    if (!LastError.isEmpty())
    {
        emit LogMessage(LastError);
        return false;
    }
    const QString ValRes = Config.Map->ValidateForAvailableHardwareChannels(Reader.CountChannels());
    if (!ValRes.isEmpty())
    {
        emit LogMessage(ValRes);
        return false;
    }

    //numProcessedEvents += Reader.CountAllProcessedEvents();
    //numBadEvents += Reader.CountBadEvents();

    // Extracting signals (or generating dummy data if disabled)
    Extractor.ClearData();
    if (Config.HldProcessSettings.bDoSignalExtraction)
    {
        emit LogAction("Extracting signals...");
        bool bOK = Extractor.ExtractSignals();
        if (!bOK)
        {
            emit LogMessage("Signal extraction failed!");
            LastError = "Signal extraction failed";
            return false;
        }
    }
    else
    {
        emit LogAction("Generating default 0 signals");
        qDebug() << "Generating default data (all signals = 0) in extractor data";
        Extractor.GenerateDummyData();
    }

    // Executing script
    if (Config.HldProcessSettings.bDoScript)
    {
        emit LogAction("Executing script...");
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
        emit LogAction("Saving to file...");
        bool bOK = SaveSignalsToFile(nameSave, false, bSaveTimeData);
        if (!bOK) return false;
    }

    //Coping data to DataHub
    if (Config.HldProcessSettings.bDoCopyToDatahub)
    {
        emit LogAction("Copying to datahub...");
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

bool AHldFileProcessor::SaveSignalsToFile(const QString FileName, bool bUseHardware, bool bSaveTimeData)
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

    if (bSaveTimeData) outStream.setRealNumberPrecision(9);

    sendSignalData(outStream, bUseHardware, bSaveTimeData);
    if (bUseHardware) emit LogAction("Signals saved using HARDWARE channels!");
    else emit LogAction("Signals saved");
    outputFile.close();
    return true;
}

void AHldFileProcessor::saveTimeData(int iEvent, QTextStream & outStream)
{
    if (iEvent < Extractor.TimeData.size())
    {
        const auto & vec = Extractor.TimeData[iEvent];

        int chanOffset = 16;
        int numChanToSave = 10;
        QVector<double> Channels(numChanToSave+1, 0);

        for (const auto & pair : vec)
        {
            //const int index = (int)pair.first - 1; // ignore "0" channel, shift all 1 down
            //if (index < 0 || index >= vec.size()) continue;
            //Channels[index] = pair.second;
            //qDebug() << pair.first << "+" << pair.second;

            int index = pair.first;
            if (index < 0) continue;
            if (index != 0)
            {
                index -= chanOffset;
                if (index > numChanToSave)
                {
                    qDebug() << "Bad channel index:"<<index;
                    continue;
                }
            }

            //qDebug() << iEvent <<"saving data:" << index << pair.second;
            Channels[index] = pair.second;
        }

        for (double time : Channels) outStream << time << " ";
    }
}

bool AHldFileProcessor::sendSignalData(QTextStream &outStream, bool bUseHardware, bool bSaveTimeData)
{
    //outStream.setRealNumberPrecision(13);

    int numEvents = Extractor.CountEvents();
    int numChannels = Extractor.CountChannels();

    if (bUseHardware)
    {
        for (int ie=0; ie<numEvents; ie++)
            if (!Extractor.IsRejectedEventFast(ie))
            {
                for (int ic=0; ic<numChannels; ic++) outStream << Extractor.GetSignalFast(ie, ic) << " ";

                if (bSaveTimeData) saveTimeData(ie, outStream);

                outStream << "\n";
            }
    }
    else
    {
        numChannels = Config.Map->CountLogicalChannels();
        for (int ie=0; ie<numEvents; ie++)
            if (!Extractor.IsRejectedEventFast(ie))
            {
                for (int ic=0; ic<numChannels; ic++) outStream << Extractor.GetSignalFast(ie, Config.Map->LogicalToHardwareFast(ic)) << " ";

                if (bSaveTimeData) saveTimeData(ie, outStream);

                outStream << "\n";
            }
    }
    return true;
}
