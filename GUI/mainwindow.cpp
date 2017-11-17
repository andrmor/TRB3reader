#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "masterconfig.h"
#include "afiletools.h"
#include "ajsontools.h"
#include "channelmapper.h"
#include "ascriptwindow.h"
#include "amessage.h"

#ifdef CERN_ROOT
#include "cernrootmodule.h"
#endif

#include "trb3datareader.h"
#include "trb3signalextractor.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QDirIterator>

#include <vector>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);    
    bStopFlag = false;
    ui->pbStop->setVisible(false);    
    qApp->processEvents();

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    Reader = new Trb3dataReader();
    Extractor = new Trb3signalExtractor(Reader);
    Map = new ChannelMapper();

    //creating master config object
    Config = new MasterConfig();

#ifdef CERN_ROOT
    RootModule = new CernRootModule(Reader, Extractor, Map, Config);
    connect(RootModule, &CernRootModule::WOneHidden, [=](){ui->pbShowWaveform->setChecked(false);});
    connect(RootModule, &CernRootModule::WOverNegHidden, [=](){ui->pbShowOverlayNeg->setChecked(false);});
    connect(RootModule, &CernRootModule::WOverPosHidden, [=](){ui->pbShowOverlayPos->setChecked(false);});
    connect(RootModule, &CernRootModule::WAllNegHidden, [=](){ui->pbShowAllNeg->setChecked(false);});
    connect(RootModule, &CernRootModule::WAllPosHidden, [=](){ui->pbShowAllPos->setChecked(false);});
#else
    QMessageBox::warning(this, "TRB3 reader", "Graph module was not configured!", QMessageBox::Ok, QMessageBox::Ok);
#endif

    //Creating script window, registering script units, and setting up QObject connections
    CreateScriptWindow();

    ui->cbAutoscaleY->setChecked(true);
    on_cobSignalExtractionMethod_currentIndexChanged(ui->cobSignalExtractionMethod->currentIndex());

    //finding the config dir
    ConfigDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/TRBreader";
    if (!QDir(ConfigDir).exists()) QDir().mkdir(ConfigDir);
    else
    {
        loadConfig(ConfigDir+"/autosave.json");

        QJsonObject jsS;
        LoadJsonFromFile(jsS, ConfigDir+"/scripting.json");
        if (!jsS.isEmpty()) ScriptWindow->ReadFromJson(jsS);
    }
    qDebug() << "-> Config dir:" << ConfigDir;

    OnEventOrChannelChanged(); // to update channel mapping indication
}

MainWindow::~MainWindow()
{
    delete Config;
    delete Map;
    delete Extractor;
    delete Reader;

#ifdef CERN_ROOT
    delete RootModule;
#endif

    delete ScriptWindow;

    delete ui;
}

void MainWindow::ClearData()
{
    Reader->ClearData();
    Extractor->ClearData();
    ui->pbSaveTotextFile->setEnabled(false);
}

void MainWindow::on_pbSelectFile_clicked()
{
    QString FileName = QFileDialog::getOpenFileName(this, "Select TRB file", "", "HLD files (*.hld)");
    if (FileName.isEmpty()) return;

    ui->leFileName->setText(FileName);
    Config->filename = FileName.toLocal8Bit().data();
    LogMessage("New file selected");
}

void MainWindow::on_pbProcessData_clicked()
{
    ui->twMain->setEnabled(false);
    LogMessage("");
    const QString error = ProcessData();

    if (error.isEmpty()) LogMessage("Processing complete");
    else
    {
        LogMessage(error);
        QMessageBox::warning(this, "TRB reader", error, QMessageBox::Ok, QMessageBox::Ok);
    }

    ui->twMain->setEnabled(true);
    ui->pbSaveTotextFile->setEnabled(true);

    OnEventOrChannelChanged();
}

const QString MainWindow::ProcessData()
{
    if (Config->filename.empty()) return "File name not defined!";

    LogMessage("Reading hld file...");
    Reader->UpdateConfig(Config);
    bool ok = Reader->Read();
    if (!ok) return "File read failed!";

    LogMessage("Extracting signals...");
    Extractor->UpdateConfig(Config);
    Extractor->ExtractSignals();
    LogMessage("Done!");
    return "";
}

void MainWindow::LogMessage(QString message)
{
    ui->leLog->setText(message);
    qApp->processEvents();
}

void MainWindow::on_pbLoadPolarities_clicked()
{
    QString FileName = QFileDialog::getOpenFileName(this, "Add channel polarity data.\n"
                                                    "The file should contain hardware channel numbers which have negative polarity");
    QVector<int> pos;
    LoadIntVectorsFromFile(FileName, &pos);

    int shift = ui->sbShiftNegatives->value();
    for (int i : pos)
        Config->NegativeChannels.push_back(i+shift);

    Extractor->ClearData();
    LogMessage("Polarities updated");

    UpdateGui();
}

void MainWindow::on_pbAddMapping_clicked()
{
    QString FileName = QFileDialog::getOpenFileName(this, "Add mapping file (cumulative!).\n"
                                                    "The file should contain hardware channel numbers sorted by logical channel number");

    QVector<int> arr;
    LoadIntVectorsFromFile(FileName, &arr);
    int shift = ui->sbShift->value();

    for (int i : arr)
        Config->ChannelMap.push_back(i+shift);

    Map->SetChannels_OrderedByLogical(Config->ChannelMap);
    Map->Validate(Reader->GetNumChannels(), true);
    LogMessage("Mapping updated");

    UpdateGui();
}

void MainWindow::on_pbAddListHardwChToIgnore_clicked()
{
    QString FileName = QFileDialog::getOpenFileName(this, "Add file with hardware channels to ignore (cumulative!).\n"
                                                    "These channels will be always digitized as having zero signal");

    QVector<int> arr;
    LoadIntVectorsFromFile(FileName, &arr);

    for (int i : arr) Config->IgnoreHardwareChannels.insert(i);

    LogMessage("Ignored channels were updated");

    UpdateGui();
}

void MainWindow::on_ptePolarity_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;

    QAction* Clear = menu.addAction("Clear");

    QAction* selectedItem = menu.exec(ui->ptePolarity->mapToGlobal(pos));
    if (!selectedItem) return;

    if (selectedItem == Clear)
    {
        Config->NegativeChannels.clear();
        ClearData();
        UpdateGui();
    }
}

void MainWindow::on_pteMapping_customContextMenuRequested(const QPoint &pos)
{
      QMenu menu;

      QAction* Validate = menu.addAction("Validate");
      menu.addSeparator();
      QAction* Clear = menu.addAction("Clear");

      QAction* selectedItem = menu.exec(ui->pteMapping->mapToGlobal(pos));
      if (!selectedItem) return;

      if (selectedItem == Validate)
        {
          if (!Reader->isValid())
          {
              QMessageBox::information(this, "TRB3 reader", "Cannot validate map - data are not loaded.", QMessageBox::Ok, QMessageBox::Ok);
              return;
          }

          bool ok = Map->Validate(Reader->GetNumChannels());
          if (ok) QMessageBox::information(this, "TRB3 reader", "Mapping is valid", QMessageBox::Ok, QMessageBox::Ok);
          else    QMessageBox::warning(this, "TRB3 reader", "mapping is NOT valid!", QMessageBox::Ok, QMessageBox::Ok);
          qDebug() << "Validation result: Map is good?"<<ok;
        }
      else if (selectedItem == Clear)
      {
          Config->ChannelMap.clear();
          Map->SetChannels_OrderedByLogical(std::vector<std::size_t>(0));
          ClearData();
          UpdateGui();
      }
}

void MainWindow::on_pteIgnoreHardwareChannels_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;

    QAction* Clear = menu.addAction("Clear");

    QAction* selectedItem = menu.exec(ui->pteIgnoreHardwareChannels->mapToGlobal(pos));
    if (!selectedItem) return;

    if (selectedItem == Clear)
    {
        Config->IgnoreHardwareChannels.clear();
        ClearData();
        UpdateGui();
    }
}

void MainWindow::on_pbSaveTotextFile_clicked()
{
    int numEvents = Extractor->GetNumEvents();
    int numChannels = Extractor->GetNumChannels();

    if (numEvents == 0 || numChannels == 0 || numEvents != Reader->GetNumEvents() || numChannels != Reader->GetNumChannels())
    {
        QMessageBox::warning(this, "TRB3reader", "Data not ready!", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    bool bUseHardware = false;
    if (!Map->Validate(numChannels))
    {
        int ret = QMessageBox::warning(this, "TRB3reader", "Channel map not valid!\nSave data without mapping (use hardware channels)?", QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) return;
        bUseHardware = true;
    }

    QString FileName = QFileDialog::getSaveFileName(this, "Save signals", "", "Data files (*.dat *.txt)");
    if (FileName.isEmpty()) return;
    if (QFileInfo(FileName).suffix().isEmpty()) FileName += ".dat";

    saveSignalsToFile(FileName, bUseHardware);
}

bool MainWindow::saveSignalsToFile(QString FileName, bool bUseHardware)
{
    QFile outputFile(FileName);
    outputFile.open(QIODevice::WriteOnly);
    if(!outputFile.isOpen())
        {
          QMessageBox::warning(this, "TRB3reader", "Unable to open file!", QMessageBox::Ok, QMessageBox::Ok);
          return false;
        }

    QTextStream outStream(&outputFile);    
    sendSignalData(outStream, bUseHardware);
    if (bUseHardware) LogMessage("Signals saved using HARDWARE channels!");
    else LogMessage("Signals saved");
    outputFile.close();
    return true;
}

bool MainWindow::sendSignalData(QTextStream &outStream, bool bUseHardware)
{
    int numEvents = Extractor->GetNumEvents();
    int numChannels = Extractor->GetNumChannels();

    if (bUseHardware)
    {
        for (int ie=0; ie<numEvents; ie++)
            if (!Extractor->IsRejectedEventFast(ie))
            {
                for (int ic=0; ic<numChannels; ic++)
                    outStream << Extractor->GetSignalFast(ie, ic) << " ";
                outStream << "\r\n";
            }
    }
    else
    {
        numChannels = Map->GetNumLogicalChannels();
        for (int ie=0; ie<numEvents; ie++)
            if (!Extractor->IsRejectedEventFast(ie))
            {
                for (int ic=0; ic<numChannels; ic++)
                    outStream << Extractor->GetSignalFast(ie, Map->LogicalToHardwareFast(ic)) << " ";
                outStream << "\r\n";
            }
    }
    return true;
}

void MainWindow::on_pbSelectNewDir_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select directory with hld files to convert", "", 0);
    if (dir.isEmpty()) return;

    ui->leDirForBulk->setText(dir);
}

void MainWindow::on_pbBulkProcess_clicked()
{
    QString dir = ui->leDirForBulk->text();
    if (dir.isEmpty())
    {
        message("Directory not selected!", this);
        return;
    }

    if (!QDir(dir).exists())
    {
        message("This directory does not exist!", this);
        return;
    }

    ui->pteBulkLog->clear();    

    ui->twMain->setEnabled(false);
    ui->pbStop->setVisible(true);
    ui->pbStop->setChecked(false);
    QDirIterator it(dir, QStringList() << "*.hld", QDir::Files, QDirIterator::Subdirectories);

    int numErrors = 0;
    while (it.hasNext())
    {
        qApp->processEvents();
        if (bStopFlag) break;

        QString fileName = it.next();
        Config->filename = fileName.toLocal8Bit().data();
        ui->pteBulkLog->appendPlainText("Processing " + QFileInfo(fileName).fileName());

        QString error = ProcessData();
        if (!error.isEmpty())
        {
            LogMessage(error);
            ui->pteBulkLog->appendPlainText("---- File reading error");
            numErrors++;
            continue;
        }

        int numEvents = Extractor->GetNumEvents();
        int numChannels = Extractor->GetNumChannels();
        if (numEvents == 0 || numChannels == 0 || numEvents != Reader->GetNumEvents() || numChannels != Reader->GetNumChannels())
        {
            ui->pteBulkLog->appendPlainText("---- Data not valid, skipped");
            numErrors++;
            continue;
        }
        if (!Map->Validate(numChannels))
        {
            ui->pteBulkLog->appendPlainText("---- Channel map not valid, skipped");
            numErrors++;
            continue;
        }

        if (ui->cbAutoExecuteScript->isChecked())
        {
            qDebug() << "Auto-executing script";
            ScriptWindow->OpenFirstTab();
            ScriptWindow->on_pbRunScript_clicked();
        }

        if (ui->cbSaveSignalsToFiles->isChecked())
        {
            QFileInfo fi(fileName);
            QString nameSave = fi.path() + "/" + fi.completeBaseName() + ui->leAddToProcessed->text();
            qDebug() << "Saving to file:"<< nameSave;
            saveSignalsToFile(nameSave, false);
        }

    }

    if (numErrors > 0) ui->pteBulkLog->appendPlainText("============= There were errors! =============");
    else ui->pteBulkLog->appendPlainText("Done - no errors");

    ui->twMain->setEnabled(true);
    ui->pbStop->setVisible(false);
    ui->pbStop->setChecked(false);
}

void MainWindow::on_pbStop_toggled(bool checked)
{
    bStopFlag = checked;
    if (checked) ui->pbStop->setText("Stopping...");
    else ui->pbStop->setText("Stop");
}

void MainWindow::on_pbShowWaveform_toggled(bool checked)
{
#ifdef CERN_ROOT
    RootModule->ShowSingleWaveWindow(checked);
    LogMessage("");
    if (!checked) return;
    if (!Reader->isValid()) return;

    int iHardwChan = getCurrentlySelectedHardwareChannel();
    if (std::isnan(iHardwChan))
    {
        RootModule->ClearSingleWaveWindow();
        return;
    }

    bool bNegative = Extractor->IsNegative(iHardwChan);
    QString s = ( bNegative ? "Neg" : "Pos");
    ui->lePolar->setText(s);

    int ievent = ui->sbEvent->value();
    if (ievent>Reader->GetNumEvents()-1)
    {
        ui->sbEvent->setValue(0);
        return;
    }

    double Min, Max;
    if (bNegative)
    {
        Min = ui->ledMinNeg->text().toDouble();
        Max = ui->ledMaxNeg->text().toDouble();
    }
    else
    {
        Min = ui->ledMinPos->text().toDouble();
        Max = ui->ledMaxPos->text().toDouble();
    }

    RootModule->DrawSingle(ievent, iHardwChan, ui->cbAutoscaleY->isChecked(), Min, Max);

#else
    QMessageBox::information(this, "", "Cern ROOT module was not configured!", QMessageBox::Ok, QMessageBox::Ok);
#endif
}

void MainWindow::on_sbEvent_valueChanged(int arg1)
{
    if (!Reader || arg1 >= Reader->GetNumEvents())
    {
        ui->sbEvent->setValue(0);
        return;
    }

    OnEventOrChannelChanged();
}

void MainWindow::on_sbChannel_valueChanged(int arg1)
{
    bool bUseLogical = (ui->cobHardwareOrLogical->currentIndex() == 1);

    if (!bUseLogical)
    {
        int max = Reader->GetNumChannels()-1;
        if (arg1 > max )
        {
            ui->sbChannel->setValue(max);
            return;
        }
    }

    OnEventOrChannelChanged(true);
}

int MainWindow::getCurrentlySelectedHardwareChannel()
{
    bool bUseLogical = (ui->cobHardwareOrLogical->currentIndex() == 1);
    int val = ui->sbChannel->value();

    int iHardwChan;

    if (bUseLogical)                          iHardwChan = Map->LogicalToHardware(val);
    else
    {
        if (val >= Reader->GetNumChannels())  iHardwChan = std::numeric_limits<size_t>::quiet_NaN();
        else                                  iHardwChan = val;
    }
    return iHardwChan;
}

void MainWindow::on_cobHardwareOrLogical_activated(int /*index*/)
{
    OnEventOrChannelChanged();
}

void MainWindow::on_cbAutoscaleY_clicked()
{
    OnEventOrChannelChanged();
}

void MainWindow::OnEventOrChannelChanged(bool bOnlyChannel)
{
    int ievent = ui->sbEvent->value();
    int val = ui->sbChannel->value();

    int iHardwChan;
    bool bUseLogical = (ui->cobHardwareOrLogical->currentIndex() == 1);
    if (bUseLogical)
    {
        ui->leLogic->setText(QString::number(val));

        iHardwChan = Map->LogicalToHardware(val);
        if (std::isnan(iHardwChan)) ui->leHardw->setText("Not mapped");
        else ui->leHardw->setText(QString::number(iHardwChan));
    }
    else
    {
        iHardwChan = val;
        ui->leHardw->setText(QString::number(val));

        std::size_t ilogical = Map->HardwareToLogical(val);
        QString s;
        if (std::isnan(ilogical)) s = "Not mapped";
        else s = QString::number(ilogical);
        ui->leLogic->setText(s);
    }

    if (Extractor->GetNumEvents() == 0)
    {
        ui->leSignal->setText("");
        return;
    }

    QString ss;
    if (Extractor->IsRejectedEventFast(ievent)) ss = "Rejected event";
    else
    {
        if (std::isnan(iHardwChan)) ss = "n.a.";
        else
        {
            double signal = Extractor->GetSignalFast(ievent, iHardwChan);
            if (std::isnan(signal)) ss = "n.a.";
            else ss = QString::number(signal);
        }
    }
    ui->leSignal->setText(ss);

    on_pbShowWaveform_toggled(ui->pbShowWaveform->isChecked());
    if (!bOnlyChannel)
    {
        on_pbShowOverlayNeg_toggled(ui->pbShowOverlayNeg->isChecked());
        on_pbShowOverlayPos_toggled(ui->pbShowOverlayPos->isChecked());
        on_pbShowAllNeg_toggled(ui->pbShowAllNeg->isChecked());
        on_pbShowAllPos_toggled(ui->pbShowAllPos->isChecked());
    }
}


void MainWindow::on_pbShowOverlayNeg_toggled(bool checked)
{
    showOverlay(checked, true);
}

void MainWindow::on_pbShowOverlayPos_toggled(bool checked)
{
    showOverlay(checked, false);
}

void MainWindow::showOverlay(bool checked, bool bNeg)
{
#ifdef CERN_ROOT
    bNeg ? RootModule->ShowOverNegWaveWindow(checked) : RootModule->ShowOverPosWaveWindow(checked);
    LogMessage("");
    if (!checked) return;
    if (!Reader->isValid()) return;

    int ievent = ui->sbEvent->value();
    if (ievent > Reader->GetNumEvents()-1)
    {
        ui->sbEvent->setValue(0);
        return;
    }

    double Min, Max;
    if (bNeg)
    {
        Min = ui->ledMinNeg->text().toDouble();
        Max = ui->ledMaxNeg->text().toDouble();
    }
    else
    {
        Min = ui->ledMinPos->text().toDouble();
        Max = ui->ledMaxPos->text().toDouble();
    }

    RootModule->DrawOverlay(ievent, bNeg, ui->cbAutoscaleY->isChecked(), Min, Max, ui->cobSortBy->currentIndex());

#else
    QMessageBox::information(this, "", "Cern ROOT module was not configured!", QMessageBox::Ok, QMessageBox::Ok);
#endif
}

void MainWindow::on_pbShowAllNeg_toggled(bool checked)
{
    showAllWave(checked, true);
}

void MainWindow::on_pbShowAllPos_toggled(bool checked)
{
    showAllWave(checked, false);
}

void MainWindow::showAllWave(bool checked, bool bNeg)
{
#ifdef CERN_ROOT
    if (!RootModule) return;
    bNeg ? RootModule->ShowAllNegWaveWindow(checked) : RootModule->ShowAllPosWaveWindow(checked);
    LogMessage("");
    if (!checked) return;
    if (!Reader->isValid()) return;

    int ievent = ui->sbEvent->value();
    if (ievent > Reader->GetNumEvents()-1)
    {
        ui->sbEvent->setValue(0);
        return;
    }

    double Min, Max;
    int padsX, padsY;
    if (bNeg)
    {
        Min = ui->ledMinNeg->text().toDouble();
        Max = ui->ledMaxNeg->text().toDouble();
        padsX = ui->sbAllNegX->value();
        padsY = ui->sbAllNegY->value();
    }
    else
    {
        Min = ui->ledMinPos->text().toDouble();
        Max = ui->ledMaxPos->text().toDouble();
        padsX = ui->sbAllPosX->value();
        padsY = ui->sbAllPosY->value();
    }

    RootModule->DrawAll(ievent, bNeg, padsX, padsY, ui->cbAutoscaleY->isChecked(), Min, Max, ui->cobSortBy->currentIndex(), ui->cbLabels->isChecked());

#else
    QMessageBox::information(this, "", "Cern ROOT module was not configured!", QMessageBox::Ok, QMessageBox::Ok);
#endif
}

void MainWindow::on_cbLabels_clicked()
{
    on_pbShowAllNeg_toggled(ui->pbShowAllNeg->isChecked());
    on_pbShowAllPos_toggled(ui->pbShowAllPos->isChecked());
}

void MainWindow::on_cobSortBy_activated(int)
{
    on_pbShowAllNeg_toggled(ui->pbShowAllNeg->isChecked());
    on_pbShowAllPos_toggled(ui->pbShowAllPos->isChecked());
}

void MainWindow::on_ledMinNeg_editingFinished()
{
    OnEventOrChannelChanged(false);
}

void MainWindow::on_ledMaxNeg_editingFinished()
{
    OnEventOrChannelChanged(false);
}

void MainWindow::on_ledMinPos_editingFinished()
{
    OnEventOrChannelChanged(false);
}

void MainWindow::on_ledMaxPos_editingFinished()
{
    OnEventOrChannelChanged(false);
}

void MainWindow::on_pbGotoNextEvent_clicked()
{
    ui->sbEvent->setValue(ui->sbEvent->value()+1);
}

void MainWindow::on_pbGotoNextChannel_clicked()
{
    ui->sbChannel->setValue(ui->sbChannel->value()+1);
}

void MainWindow::on_sbAllNegX_editingFinished()
{
    on_pbShowAllNeg_toggled(ui->pbShowAllNeg->isChecked());
}

void MainWindow::on_sbAllNegY_editingFinished()
{
    on_pbShowAllNeg_toggled(ui->pbShowAllNeg->isChecked());
}

void MainWindow::on_sbAllPosX_editingFinished()
{
    on_pbShowAllPos_toggled(ui->pbShowAllPos->isChecked());
}

void MainWindow::on_sbAllPosY_editingFinished()
{
    on_pbShowAllPos_toggled(ui->pbShowAllPos->isChecked());
}

void MainWindow::on_cbSubstractPedestal_toggled(bool)
{
    updateSmoothAfterPedeEnableStatus();
}

void MainWindow::on_cbSmoothWaveforms_toggled(bool)
{
    updateSmoothAfterPedeEnableStatus();
}

void MainWindow::updateSmoothAfterPedeEnableStatus()
{
    ui->cbSmoothBeforePedestal->setEnabled(ui->cbSubstractPedestal->isChecked() && ui->cbSmoothWaveforms->isChecked());
}

void MainWindow::on_actionReset_positions_of_all_windows_triggered()
{
    setGeometry(10,10,600,800);
    ScriptWindow->setGeometry(670,10,600,800);

#ifdef CERN_ROOT
    RootModule->ResetPositionOfWindows();
#endif
}

void MainWindow::on_pbNegSignature_clicked()
{
#ifdef CERN_ROOT
    if (!RootModule) return;
    LogMessage("");
    if (!Reader->isValid()) return;

    RootModule->DrawSignature(true);

#else
    QMessageBox::information(this, "", "Cern ROOT module was not configured!", QMessageBox::Ok, QMessageBox::Ok);
#endif
}

void MainWindow::on_pbPosSignature_clicked()
{
#ifdef CERN_ROOT
    if (!RootModule) return;
    LogMessage("");
    if (!Reader->isValid()) return;

    RootModule->DrawSignature(false);

#else
    QMessageBox::information(this, "", "Cern ROOT module was not configured!", QMessageBox::Ok, QMessageBox::Ok);
#endif
}

void MainWindow::on_cobSignalExtractionMethod_currentIndexChanged(int index)
{
    ui->sbExtractAllFromSampleNumber->setVisible(index == 2);
}
