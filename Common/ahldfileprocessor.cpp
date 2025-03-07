#include "ahldfileprocessor.h"
#include "masterconfig.h"
#include "trb3datareader.h"
#include "trb3signalextractor.h"
#include "adatahub.h"
#include "channelmapper.h"

#include <QFileInfo>
#include <QDebug>

AHldFileProcessor::AHldFileProcessor(Trb3dataReader & Reader, Trb3signalExtractor & Extractor, ADataHub & DataHub) :
    Config(MasterConfig::getConstInstance()), Reader(Reader), Extractor(Extractor), DataHub(DataHub) {}

bool AHldFileProcessor::ProcessFile(const QString FileName, int What_0signals1waves, bool bIncludeTimeData, const QString SaveFileName, bool doNotSaveSuppressedChannels)
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
            QString extra = Config.HldProcessSettings.AddToFileName;
            if (Config.HldProcessSettings.AddRunTime)
            {
                long runDuration = Reader.timeOfStart.secsTo(Reader.timeOfEnd);
                extra = "_" + QString::number(runDuration) + extra;
            }
            nameSave = fi.path() + "/" + fi.completeBaseName() + extra;
        }
        else nameSave = SaveFileName;

        qDebug() << "Saving to file:"<< nameSave;
        emit LogAction("Saving to file...");

        if (What_0signals1waves == 0)
        {
            bool bOK = SaveSignalsToFile(nameSave, false, bIncludeTimeData, doNotSaveSuppressedChannels);
            if (!bOK) return false;
        }
        else if (What_0signals1waves == 1)
        {
            bool bOK = SaveWaveformsToFile(nameSave, false, bIncludeTimeData, doNotSaveSuppressedChannels);
            if (!bOK) return false;
        }
        else
        {
            QString err = "Unknown option in AHldFileProcessor data processing: What_0signals1waves should be 0 or 1";
            emit LogMessage(err);
            LastError = err;
            return false;
        }
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

bool AHldFileProcessor::SaveSignalsToFile(const QString & FileName, bool bUseHardware, bool bSaveTimeData, bool doNotSaveSuppressed)
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

    sendSignalData(outStream, bUseHardware, bSaveTimeData, doNotSaveSuppressed);
    if (bUseHardware) emit LogAction("Signals saved using HARDWARE channels!");
    else emit LogAction("Signals saved");
    outputFile.close();
    return true;
}

bool AHldFileProcessor::SaveWaveformsToFile(const QString & FileName, bool bUseHardware, bool bSaveTimeData, bool doNotSaveSuppressed)
{
    QFile outputFile(FileName);
    outputFile.open(QIODevice::WriteOnly);
    if(!outputFile.isOpen())
    {
        emit LogMessage("Failed to save waveforms");
        LastError = "Unable to open file to save waveforms: " + FileName;
        return false;
    }

    QTextStream outStream(&outputFile);

    sendWaveformData(outStream, bUseHardware, bSaveTimeData, doNotSaveSuppressed);
    if (bUseHardware) emit LogAction("Waveforms saved using HARDWARE channels!");
    else emit LogAction("Waveforms saved");
    outputFile.close();
    return true;
}

void AHldFileProcessor::saveTimeData(int iEvent, QTextStream & outStream)
{
    if (iEvent < Reader.timeData.size())
    {
        for (const Trb3TimingRecord & chRec : Reader.timeData[iEvent])
        {
            outStream << chRec.TimingCannel << " ";
            for (double d : chRec.Triggers)
                outStream << d << " ";
            outStream << '\n';
        }
    }
}

bool AHldFileProcessor::sendSignalData(QTextStream &outStream, bool bUseHardware, bool bSaveTimeData, bool doNotSaveSuppressed)
{
    if (bSaveTimeData) outStream.setRealNumberPrecision(13);

    int numEvents = Extractor.CountEvents();
    int numChannels = Extractor.CountChannels();

    if (bUseHardware)
    {
        for (int ie=0; ie<numEvents; ie++)
            if (!Extractor.IsRejectedEventFast(ie))
            {
                for (int ic=0; ic<numChannels; ic++)
                {
                    //if (doNotSaveSuppressed && Config.IsIgnoredHardwareChannel(ic)) continue; // will be confusing!
                    outStream << Extractor.GetSignalFast(ie, ic) << " ";
                }

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
                for (int ic=0; ic<numChannels; ic++)
                {
                    if (doNotSaveSuppressed && Config.IsIgnoredLogicalChannel(ic)) continue;
                    outStream << Extractor.GetSignalFast(ie, Config.Map->LogicalToHardwareFast(ic)) << " ";
                }

                if (bSaveTimeData) saveTimeData(ie, outStream);

                outStream << "\n";
            }
    }
    return true;
}

bool AHldFileProcessor::sendWaveformData(QTextStream &outStream, bool bUseHardware, bool bSaveTimeData, bool doNotSaveSuppressed)
{
    if (bSaveTimeData) outStream.setRealNumberPrecision(13);

    int numEvents = Reader.CountEvents();
    int numChannels = ( bUseHardware ? Reader.CountChannels() : Config.Map->CountLogicalChannels() );

    for (int ie = 0; ie < numEvents; ie++)
    {
        outStream << "#" << ie << "\n";

        for (int ic = 0; ic < numChannels; ic++)
        {
            int ihardwchan;
            if (bUseHardware) ihardwchan = ic;
            else
            {
                ihardwchan = Config.Map->LogicalToHardware(ic);
                if (doNotSaveSuppressed && Config.IsIgnoredHardwareChannel(ihardwchan)) continue;
            }

            const QVector<float> * waves = Reader.GetWaveformPtrFast(ie, ihardwchan);
            for (int i = 0; i < waves->size(); i++)
                outStream << waves->at(i) << " ";
            outStream << '\n';
        }

        if (bSaveTimeData)
        {
            saveTimeData(ie, outStream);
            //outStream << '\n';
        }
    }

    return true;
}
