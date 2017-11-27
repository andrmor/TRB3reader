#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "masterconfig.h"
#include "afiletools.h"
#include "ajsontools.h"
#include "channelmapper.h"
#include "ascriptwindow.h"
#include "amessage.h"
#include "adispatcher.h"
#include "aeditchannelsdialog.h"
#include "adatahub.h"

#ifdef CERN_ROOT
#include "cernrootmodule.h"
#endif

#include "trb3datareader.h"
#include "trb3signalextractor.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>
#include <QInputDialog>

//#include <vector>
#include <cmath>

MainWindow::MainWindow(ADataHub *DataHub, QWidget *parent) :
    QMainWindow(parent), DataHub(DataHub),
    ui(new Ui::MainWindow)
{
    Dispatcher = 0;
    ui->setupUi(this);    
    bStopFlag = false;
    ui->pbStop->setVisible(false);    
    qApp->processEvents();

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    //creating master config object
    Config = new MasterConfig();

    Reader = new Trb3dataReader(Config);
    Extractor = new Trb3signalExtractor(Config, Reader);

#ifdef CERN_ROOT
    RootModule = new CernRootModule(Reader, Extractor, Config);
    connect(RootModule, &CernRootModule::WOneHidden, [=](){ui->pbShowWaveform->setChecked(false);});
    connect(RootModule, &CernRootModule::WOverNegHidden, [=](){ui->pbShowOverlayNeg->setChecked(false);});
    connect(RootModule, &CernRootModule::WOverPosHidden, [=](){ui->pbShowOverlayPos->setChecked(false);});
    connect(RootModule, &CernRootModule::WAllNegHidden, [=](){ui->pbShowAllNeg->setChecked(false);});
    connect(RootModule, &CernRootModule::WAllPosHidden, [=](){ui->pbShowAllPos->setChecked(false);});
#else
    QMessageBox::warning(this, "TRB3 reader", "Graph module was not configured!", QMessageBox::Ok, QMessageBox::Ok);
#endif

    Dispatcher = new ADispatcher(Config, Reader, Extractor, this); //also loads config if autosave exists

    //Creating script window, registering script units, and setting up QObject connections
    CreateScriptWindow();

    //Loading window settings
    QJsonObject json;
    LoadJsonFromFile(json, Dispatcher->AutosaveFile);
    if (!json.isEmpty())
    {
        readWindowsFromJson(json);

        QJsonObject jsS;
        LoadJsonFromFile(jsS, Dispatcher->ConfigDir+"/scripting.json");
        if (!jsS.isEmpty())
            ScriptWindow->ReadFromJson(jsS);
    }

    //misc gui settings
    ui->cbAutoscaleY->setChecked(true);
    on_cobSignalExtractionMethod_currentIndexChanged(ui->cobSignalExtractionMethod->currentIndex());
    OnEventOrChannelChanged(); // to update channel mapping indication
}

MainWindow::~MainWindow()
{
    delete Config;
    delete Extractor;
    delete Reader;

#ifdef CERN_ROOT
    delete RootModule;
#endif

    delete ScriptWindow;

    delete ui;
}

void MainWindow::on_pbSelectFile_clicked()
{
    QString FileName = QFileDialog::getOpenFileName(this, "Select TRB file", Config->WorkingDir, "HLD files (*.hld)");
    if (FileName.isEmpty()) return;
    Config->WorkingDir = QFileInfo(FileName).absolutePath();

    ui->leFileName->setText(FileName);
    Config->FileName = FileName;
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
    if (Config->FileName.isEmpty()) return "File name not defined!";

    LogMessage("Reading hld file...");
    bool ok = Reader->Read();
    if (!ok) return "File read failed!";

    LogMessage("Extracting signals...");
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
    QVector<int> negList;
    LoadIntVectorsFromFile(FileName, &negList);


    Config->SetNegativeChannels(negList);

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

    for (int i : arr)
    {
        if (i<0)
        {
            message("Only positive channel numbers are allowed!", this);
            return;
        }
    }
    Config->SetMapping(arr);

    Config->Map->Validate(Reader->GetNumChannels(), true);
    LogMessage("Mapping updated");

    UpdateGui();
}

void MainWindow::on_pbAddListHardwChToIgnore_clicked()
{
    QString FileName = QFileDialog::getOpenFileName(this, "Add file with hardware channels to ignore (cumulative!).\n"
                                                    "These channels will be always digitized as having zero signal");

    QVector<int> arr;
    LoadIntVectorsFromFile(FileName, &arr);

    Config->SetListOfIgnoreChannels(arr);

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
        Dispatcher->ClearNegativeChannels();
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

          bool ok = Config->Map->Validate(Reader->GetNumChannels());
          if (ok) QMessageBox::information(this, "TRB3 reader", "Mapping is valid", QMessageBox::Ok, QMessageBox::Ok);
          else    QMessageBox::warning(this, "TRB3 reader", "mapping is NOT valid!", QMessageBox::Ok, QMessageBox::Ok);
          qDebug() << "Validation result: Map is good?"<<ok;
        }
      else if (selectedItem == Clear)
          Dispatcher->ClearMapping();
}

void MainWindow::on_pteIgnoreHardwareChannels_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu;

    QAction* Clear = menu.addAction("Clear");

    QAction* selectedItem = menu.exec(ui->pteIgnoreHardwareChannels->mapToGlobal(pos));
    if (!selectedItem) return;

    if (selectedItem == Clear)
        Dispatcher->ClearIgnoreChannels();
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
    if (!Config->Map->Validate(numChannels))
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
        numChannels = Config->Map->GetNumLogicalChannels();
        for (int ie=0; ie<numEvents; ie++)
            if (!Extractor->IsRejectedEventFast(ie))
            {
                for (int ic=0; ic<numChannels; ic++)
                    outStream << Extractor->GetSignalFast(ie, Config->Map->LogicalToHardwareFast(ic)) << " ";
                outStream << "\r\n";
            }
    }
    return true;
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
    if ( iHardwChan < 0 )
    {
        RootModule->ClearSingleWaveWindow();
        return;
    }

    bool bNegative = Config->IsNegative(iHardwChan);
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

    if (bUseLogical)                          iHardwChan = Config->Map->LogicalToHardware(val);
    else
    {
        if (val >= Reader->GetNumChannels())  iHardwChan = -1;
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

        iHardwChan = Config->Map->LogicalToHardware(val);
        if ( iHardwChan < 0 ) ui->leHardw->setText("n.a.");
        else ui->leHardw->setText(QString::number(iHardwChan));
    }
    else
    {
        iHardwChan = val;
        ui->leHardw->setText(QString::number(val));

        int ilogical = Config->Map->HardwareToLogical(val);
        QString s;
        if ( ilogical < 0 ) s = "n.a.";
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
        if ( iHardwChan < 0 ) ss = "n.a.";
        else
        {
            double signal = Extractor->GetSignalFast(ievent, iHardwChan);
            if ( std::isnan(signal) ) ss = "n.a.";
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

void MainWindow::ClearData()
{
    Dispatcher->ClearData();
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

bool MainWindow::ExtractNumbersFromQString(const QString input, QVector<int> *ToAdd)
{
  ToAdd->clear();

  QRegExp rx("(\\,|\\-)"); //RegEx for ' ' and '-'

  QStringList fields = input.split(rx, QString::SkipEmptyParts);

  if (fields.size() == 0 )
    {
      //message("Nothing to add!");
      return false;
    }

  fields = input.split(",", QString::SkipEmptyParts);
  //  qDebug()<<"found "<<fields.size()<<" records";

  for (int i=0; i<fields.size(); i++)
    {
      QString thisField = fields[i];

      //are there "-" separated fields?
      QStringList subFields = thisField.split("-", QString::SkipEmptyParts);

      if (subFields.size() > 2 || subFields.size() == 0) return false;
      else if (subFields.size() == 1)
        {
          //just one number
          bool ok;
          int pm;
          pm = subFields[0].toInt(&ok);

          if (ok) ToAdd->append(pm);
          else return false;
        }
      else //range - two subFields
        {
          bool ok1, ok2;
          int pm1, pm2;
          pm1 = subFields[0].toInt(&ok1);
          pm2 = subFields[1].toInt(&ok2);
          if (ok1 && ok2)
            {
               if (pm2<pm1) return false;//error = true;
               else
                 {
                   for (int j=pm1; j<=pm2; j++) ToAdd->append(j);
                 }
            }
          else return false;
        }
    }

  return true;
}

QString MainWindow::PackChannelList(QVector<int> vec)
{
    if (vec.isEmpty()) return "";

    std::sort(vec.begin(), vec.end());

    QString out;
    int prevVal = vec.first();
    int rangeStart = prevVal;
    for (int i=0; i<=vec.size(); ++i)        //includes the invalid index!
    {
        int thisVal;
        if ( i == vec.size() )               //last in the vector
        {
            thisVal = prevVal;
        }
        else
        {
            thisVal = vec.at(i);
            if ( i == 0 )                    // first but not the only
            {
                continue;
            }
            else if ( thisVal == prevVal+1 ) //continuing range
            {
                prevVal++;
                continue;
            }
        }

        //adding to output
        if (!out.isEmpty()) out += ", ";

        if (prevVal == rangeStart)             //single val
            out += QString::number(prevVal);
        else                                //range ended
            out += QString::number(rangeStart) + "-" + QString::number(prevVal);

        prevVal = thisVal;
        rangeStart = thisVal;
    }

    return out;
}

void MainWindow::on_pbEditListOfNegatives_clicked()
{
    QString old =  PackChannelList(Config->GetListOfNegativeChannels());
    AEditChannelsDialog* D = new AEditChannelsDialog("List of negative channels", old, "Example: 0, 2, 5-15, 7, 30-45");
    int res = D->exec();
    if (res != 1) return;
    const QString str = D->GetText();
    delete D;

    QVector<int> vec;
    ExtractNumbersFromQString(str, &vec);
    QSet<int> set;
    for (int i : vec) set << i;

    vec.clear();
    for (int i: set) vec << i;

    Config->SetNegativeChannels(vec);
    UpdateGui();
}

void MainWindow::on_pbEditMap_clicked()
{
    QString old;
    for (int i : Config->GetMapping()) old += QString::number(i) + " ";

    AEditChannelsDialog* D = new AEditChannelsDialog("Hardware channels sorted by logical number", old, "Example: 5 4 3 2 1 0 10 11 12");
    int res = D->exec();
    if (res != 1) return;
    const QString str = D->GetText().simplified();
    delete D;

    QRegExp rx("(\\ |\\,|\\:|\\t|\\n)");
    QStringList fields = str.split(rx, QString::SkipEmptyParts);
    QVector<int> vec;
    for (QString str : fields)
    {
        bool bOK;
        int i = str.toInt(&bOK);
        if (!bOK || i<0)
        {
            message("Error in format: only integers (>=0) are accepted", this);
            return;
        }
        vec << i;
    }

    Config->SetMapping(vec);
    UpdateGui();
}

void MainWindow::on_pbEditIgnoreChannelList_clicked()
{
    QString old =  PackChannelList(Config->GetListOfIgnoreChannels());
    AEditChannelsDialog* D = new AEditChannelsDialog("List of ignored hardware channels", old, "Example: 2, 5-15, 30-45");
    int res = D->exec();
    if (res != 1) return;
    const QString str = D->GetText();
    delete D;

    QVector<int> vec;
    ExtractNumbersFromQString(str, &vec);
    QSet<int> set;
    for (int i : vec) set << i;

    vec.clear();
    for (int i: set) vec << i;

    Config->SetListOfIgnoreChannels(vec);
    UpdateGui();
}

void MainWindow::on_pbAddDatakind_clicked()
{
    bool bOK;
    int datakind = QInputDialog::getInt(this, "TRBreader", "Input new datakind to add", 0, 0, 0xFFFF, 1, &bOK);
    if (bOK)
        Config->AddDatakind(datakind);
    UpdateGui();
}

void MainWindow::on_pbRemoveDatakind_clicked()
{
    int raw = ui->lwDatakinds->currentRow();
    if (raw < 0)
    {
        message("Select datakind to remove by left-clicking on it in the list above", this);
        return;
    }
    QString sel = ui->lwDatakinds->currentItem()->text();
    QStringList sl = sel.split(" ");
    if (sl.size()>1)
    {
        QString dk = sl.first();
        int datakind = dk.toInt();
        Config->RemoveDatakind(datakind);
    }
    UpdateGui();
}

void MainWindow::on_pbPrintHLDfileProperties_clicked()
{
    QString FileName = QFileDialog::getOpenFileName(this, "Select HLD file to inspect", Config->WorkingDir, "*.hld");
    if (FileName.isEmpty()) return;
    Config->WorkingDir = QFileInfo(FileName).absolutePath();

    QString s = Reader->GetFileInfo(FileName);
    ui->pteHLDfileProperties->clear();
    ui->pteHLDfileProperties->appendPlainText(s);
}

void MainWindow::on_pbProcessAllFromDir_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select directory with hld files to convert", Config->WorkingDir, 0);
    if (dir.isEmpty()) return;
    Config->WorkingDir = QFileInfo(dir).absolutePath();

    if (!QDir(dir).exists())
    {
        message("This directory does not exist!", this);
        return;
    }

    QStringList names;
    QDirIterator it(dir, QStringList() << "*.hld", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) names << it.next();

    bulkProcessorEnvelope(names);
}

void MainWindow::on_pbProcessSelectedFiles_clicked()
{
    QStringList names = QFileDialog::getOpenFileNames(this, "Select hld files to be processed", Config->WorkingDir, "*.hld");
    if (names.isEmpty()) return;
    Config->WorkingDir = QFileInfo(names.first()).absolutePath();

    bulkProcessorEnvelope(names);
}

void MainWindow::bulkProcessorEnvelope(QStringList FileNames)
{
    if (!ui->cbKeepEvents->isChecked()) DataHub->Clear();
    ui->pteBulkLog->clear();
    if (ui->cbAutoExecuteScript->isChecked())
    {
        ScriptWindow->show();
        ScriptWindow->OpenFirstTab();
    }

    ui->twMain->setEnabled(false);
    ui->pbStop->setVisible(true);
    ui->pbStop->setChecked(false);

    int numErrors = 0;
    for (QString name : FileNames)
    {
        Config->FileName = name;

        bool bOK = bulkProcessCore();
        if (!bOK) numErrors++;

        updateNumEventsIndication();
        qApp->processEvents();
        if (bStopFlag) break;
    }

    if (numErrors > 0) ui->pteBulkLog->appendPlainText("============= There were errors! =============");
    else ui->pteBulkLog->appendPlainText("Done - no errors");

    ui->twMain->setEnabled(true);
    ui->pbStop->setVisible(false);
    ui->pbStop->setChecked(false);

    UpdateGui();
}

bool MainWindow::bulkProcessCore()
{
    if (Config->FileName.isEmpty())
    {
        ui->pteBulkLog->appendPlainText("---- File name not defined!");
        return false;
    }
    LogMessage("Reading hld file...");
    ui->pteBulkLog->appendPlainText("Processing " + QFileInfo(Config->FileName).fileName());
    qDebug() << "Processing" <<  Config->FileName;

    // Reading waveforms, pefroming optional smoothing/pedestal substraction
    bool ok = Reader->Read();
    if (!ok)
    {
        ui->pteBulkLog->appendPlainText("---- File read failed!");
        return false;
    }

    // Extracting signals (or generating dummy data if disabled)
    Extractor->ClearData();
    if (ui->cbBulkExtract->isChecked())
    {
        LogMessage("Extracting signals...");
        bool bOK = Extractor->ExtractSignals();
        if (!bOK)
        {
            ui->pteBulkLog->appendPlainText("---- Signal extraction failed!");
            return false;
        }
    }
    else
    {
        qDebug() << "generatin gdefault data (all signals = 0) in extractor data";
        Extractor->GenerateDummyData();
    }

    // Executing script
    if (ui->cbAutoExecuteScript->isChecked())
    {
        LogMessage("Executing script...");
        bool bOK = ScriptWindow->ExecuteScriptInFirstTab();
        if (!bOK)
        {
            ui->pteBulkLog->appendPlainText("---- Script execution error");
            return false;
        }
    }

    // Checking that after extraction/script the data are consistent in num channels / mapping
    int numEvents = Extractor->GetNumEvents();
    int numChannels = Extractor->GetNumChannels();
    if (numEvents == 0 || numChannels == 0 || numEvents != Reader->GetNumEvents() || numChannels != Reader->GetNumChannels())
    {
        ui->pteBulkLog->appendPlainText("---- Extractor data not valid -> ignoring this file");
        return false;
    }
    if (!Config->Map->Validate(numChannels))
    {
        ui->pteBulkLog->appendPlainText("---- Conflict with channel map -> ignoring this file");
        return false;
    }

    // saving to individual files
    if (ui->cbSaveSignalsToFiles->isChecked())
    {
        QFileInfo fi(Config->FileName);
        QString nameSave = fi.path() + "/" + fi.completeBaseName() + ui->leAddToProcessed->text();
        qDebug() << "Saving to file:"<< nameSave;
        saveSignalsToFile(nameSave, false);
    }

    //Coping data to DataHub
    if (ui->cbBulkCopyToDatahub->isChecked())
    {
        qDebug() << "Copying to DataHub...";
        const QVector<int>& map = Config->Map->GetMapToHardware();

        for (int iev=0; iev<numEvents; iev++)
        {
            if (Extractor->IsRejectedEventFast(iev)) continue;

            AOneEvent* ev = new AOneEvent;

            //signals
            const QVector<float>* vecHardw = Extractor->GetSignalsFast(iev);
            QVector<float> vecLogical;
            for (int ihardw : map) vecLogical << vecHardw->at(ihardw);
            ev->SetSignals(&vecLogical);

            //rejection status
            ev->SetRejectedFlag(false);

            //waveforms
            if (ui->cbBulkAlsoCopyWaveforms->isChecked())
            {
                // to do!!!
            }

            DataHub->AddEvent(ev);
        }
    }

    return true;
}

void MainWindow::on_pbSaveSignalsFromDataHub_clicked()
{
    int numEvents = DataHub->CountEvents();
    if ( numEvents == 0)
    {
        message("There are no events in the DataHub", this);
        return;
    }

    QString FileName = QFileDialog::getSaveFileName(this, "Save events from DataHub", Config->WorkingDir, "*.dat,*.txt");
    if (FileName.isEmpty()) return;
    Config->WorkingDir = QFileInfo(FileName).absolutePath();

    QFile outFile( FileName );
    outFile.open(QIODevice::WriteOnly);
    if(!outFile.isOpen())
      {
        message("Unable to open file " +FileName+ " for writing!", this);
        return;
      }
    QTextStream outStream(&outFile);

    for (int iev=0; iev<numEvents; iev++)
    {
        const QVector<float>* vec = DataHub->GetSignalsFast(iev);
        for (float val : *vec) outStream << QString::number(val) << " ";
        //if (ui->cbAddReconstructedPositions->isChecked())
        // *** !!!
        outStream << "\r\n";
    }
}
