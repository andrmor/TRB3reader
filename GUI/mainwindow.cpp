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
#include <QProgressBar>

#include <cmath>

MainWindow::MainWindow(MasterConfig* Config, ADispatcher *Dispatcher, ADataHub* DataHub, Trb3dataReader* Reader, Trb3signalExtractor* Extractor, QWidget *parent) :
    QMainWindow(parent),
    Config(Config), Dispatcher(Dispatcher), DataHub(DataHub), Reader(Reader), Extractor(Extractor),
    ui(new Ui::MainWindow)
{
    bStopFlag = false;
    ui->setupUi(this);

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

#ifdef CERN_ROOT
    RootModule = new CernRootModule(Reader, Extractor, Config, DataHub);
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

    //Loading window settings
    LoadWindowSettings();
    QJsonObject jsS;
    LoadJsonFromFile(jsS, Dispatcher->ConfigDir+"/scripting.json");
    if (!jsS.isEmpty()) ScriptWindow->ReadFromJson(jsS);

    //misc gui settings
    ui->prbMainBar->setVisible(false);
    ui->cbAutoscaleY->setChecked(true);
    ui->pbStop->setVisible(false);
    on_cobSignalExtractionMethod_currentIndexChanged(ui->cobSignalExtractionMethod->currentIndex());
    OnEventOrChannelChanged(); // to update channel mapping indication
}

MainWindow::~MainWindow()
{
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
    on_sbEvent_valueChanged(ui->sbEvent->value());
    updateNumEventsIndication();
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

void MainWindow::LogMessage(const QString message)
{
    ui->leLog->setText(message);
    qApp->processEvents();
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

void MainWindow::on_pbLoadPolarities_clicked()
{
    QString FileName = QFileDialog::getOpenFileName(this, "Add channel polarity data.\n"
                                                    "The file should contain hardware channel numbers which have negative polarity");
    QVector<int> negList;
    //LoadIntVectorsFromFile(FileName, &negList);
    QString AllText;
    bool bOK = LoadTextFromFile(FileName, AllText);
    if (!bOK)
    {
        message("Read failed!", this);
        return;
    }
    AllText = AllText.simplified();
    QRegExp rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'
    QStringList sl = AllText.split(rx, QString::SkipEmptyParts);

    bool bStrangies = false;
    for (QString& s : sl)
    {
        bool bOK;
        int val = s.toInt(&bOK);
        if (bOK && val>=0) negList << val;
        else
        {
            bStrangies = true;
            continue;
        }
    }

    if (bStrangies) message("There were some unexpected fields in the file which were ignored!", this);

    Config->SetNegativeChannels(negList);

    ClearData();
    LogMessage("Polarities updated");

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

    bool bOK = Config->SetMapping(vec);
    if (!bOK) message("Ignored: there are non-unique channel numbers in the list!", this);

    UpdateGui();
}

void MainWindow::on_pbAddMapping_clicked()
{
    QString FileName = QFileDialog::getOpenFileName(this, "Add mapping file (cumulative!).\n"
                                                    "The file should contain hardware channel numbers sorted by logical channel number");

    QVector<int> arr;
    //LoadIntVectorsFromFile(FileName, &arr);

    QString AllText;
    bool bOK = LoadTextFromFile(FileName, AllText);
    if (!bOK)
    {
        message("Read failed!", this);
        return;
    }
    AllText = AllText.simplified();
    QRegExp rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'
    QStringList sl = AllText.split(rx, QString::SkipEmptyParts);

    bool bStrangies = false;
    for (QString& s : sl)
    {
        bool bOK;
        int val = s.toInt(&bOK);
        if (bOK && val>=0) arr << val;
        else
        {
            bStrangies = true;
            continue;
        }
    }

    if (bStrangies) message("There were some unexpected fields in the file which were ignored!", this);

    bOK = Config->SetMapping(arr);
    if (bOK)
    {
        //Config->Map->Validate(Reader->CountChannels(), true);
        LogMessage("Mapping updated");
    }
    else message("Ignored: There are non-unique channel numbers in the list!", this);

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

void MainWindow::on_pbAddListHardwChToIgnore_clicked()
{
    QString FileName = QFileDialog::getOpenFileName(this, "Add file with hardware channels to ignore (cumulative!).\n"
                                                    "These channels will be always digitized as having zero signal");

    QVector<int> arr;
    //LoadIntVectorsFromFile(FileName, &arr);

    QString AllText;
    bool bOK = LoadTextFromFile(FileName, AllText);
    if (!bOK)
    {
        message("Read failed!", this);
        return;
    }
    AllText = AllText.simplified();
    QRegExp rx("(\\ |\\,|\\:|\\t)"); //separators: ' ' or ',' or ':' or '\t'
    QStringList sl = AllText.split(rx, QString::SkipEmptyParts);

    bool bStrangies = false;
    for (QString& s : sl)
    {
        bool bOK;
        int val = s.toInt(&bOK);
        if (bOK && val>=0) arr << val;
        else
        {
            bStrangies = true;
            continue;
        }
    }

    if (bStrangies) message("There were some unexpected fields in the file which were ignored!", this);

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

#include <QDialog>
void MainWindow::on_pteMapping_customContextMenuRequested(const QPoint &pos)
{
      QMenu menu;

      QAction* Validate = menu.addAction("Validate");
      menu.addSeparator();
      QAction* Clear = menu.addAction("Clear");
      menu.addSeparator();
      QAction* PrintToLogical = menu.addAction("Print hardware->logical map");
      QAction* PrintToHardware = menu.addAction("Print logical->hardware map");

      QAction* selectedItem = menu.exec(ui->pteMapping->mapToGlobal(pos));
      if (!selectedItem) return;

      if (selectedItem == Validate)
        {
          const QString err = Config->Map->Validate();
          QString output;
          if (err.isEmpty()) output = "Map is valid";
          else output = "Map is NOT valid:\n" + err;

          QMessageBox::information(this, "TRB3 reader", output, QMessageBox::Ok, QMessageBox::Ok);
          qDebug() << "Validation result:"<<output;
        }
      else if (selectedItem == Clear) Dispatcher->ClearMapping();
      else if (selectedItem == PrintToLogical || selectedItem == PrintToHardware)
      {
          QStringList list;
          QString title;
          if (selectedItem == PrintToLogical)
          {
              list = Config->Map->PrintToLogical();
              title = "Hardware -> Logical";
          }
          else
          {
              list = Config->Map->PrintToHardware();
              title = "Logical -> Hardware";
          }

          QDialog* D = new QDialog(this);
          D->setWindowTitle("Channel mapping");
          QVBoxLayout* l = new QVBoxLayout(D);

            QLabel* lab = new QLabel(title);
            l->addWidget(lab);

            QListWidget* lw = new QListWidget();
            lw->addItems(list);
            l->addWidget(lw);

          D->resize(200,400);
          D->exec();
      }
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
    int numEvents = Extractor->CountEvents();
    int numChannels = Extractor->CountChannels();

    if (numEvents == 0 || numChannels == 0 || numEvents != Reader->CountEvents() || numChannels != Reader->CountChannels())
    {
        QMessageBox::warning(this, "TRB3reader", "Data not ready!", QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    bool bUseHardware = false;
    if (!Config->Map->Validate().isEmpty())
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

bool MainWindow::saveSignalsToFile(const QString FileName, bool bUseHardware)
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
    int numEvents = Extractor->CountEvents();
    int numChannels = Extractor->CountChannels();

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
        numChannels = Config->CountLogicalChannels();
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

void MainWindow::on_sbEvent_valueChanged(int arg1)
{
    if (!Reader || arg1 >= Reader->CountEvents())
    {
        ui->sbEvent->setValue(0);
        return;
    }

    OnEventOrChannelChanged();

    on_pbShowWaveform_toggled(ui->pbShowWaveform->isChecked());
    on_pbShowOverlayNeg_toggled(ui->pbShowOverlayNeg->isChecked());
    on_pbShowOverlayPos_toggled(ui->pbShowOverlayPos->isChecked());
    on_pbShowAllNeg_toggled(ui->pbShowAllNeg->isChecked());
    on_pbShowAllPos_toggled(ui->pbShowAllPos->isChecked());
}

void MainWindow::on_sbChannel_valueChanged(int)
{
    OnEventOrChannelChanged();

    on_pbShowWaveform_toggled(ui->pbShowWaveform->isChecked());
}

int MainWindow::getCurrentlySelectedHardwareChannel()
{
    bool bUseLogical = (ui->cobHardwareOrLogical->currentIndex() == 1);
    int val = ui->sbChannel->value();

    int iHardwChan;

    if (bUseLogical)                          iHardwChan = Config->Map->LogicalToHardware(val);
    else
    {
        if (val >= Reader->CountChannels())  iHardwChan = -1;
        else                                  iHardwChan = val;
    }
    return iHardwChan;
}

void MainWindow::on_cobHardwareOrLogical_activated(int /*index*/)
{
    OnEventOrChannelChanged();
    on_sbEvent_valueChanged(ui->sbEvent->value());
}

void MainWindow::on_cbAutoscaleY_clicked()
{
    OnEventOrChannelChanged();
    on_sbEvent_valueChanged(ui->sbEvent->value());
}

void MainWindow::OnEventOrChannelChanged()
{
    int ievent = ui->sbEvent->value();
    int val = ui->sbChannel->value();

    int iHardwChan;
    bool bFromDataHub = (ui->cobExplorerSource->currentIndex()==1);
    bool bUseLogical = (bFromDataHub || ui->cobHardwareOrLogical->currentIndex()==1);

    // Update channel indication
    if (bUseLogical)
    {
        ui->leLogic->setText(QString::number(val));
        if (val>=Config->CountLogicalChannels())
        {
            ui->sbChannel->setValue(0);
            return;
        }

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

    bool bNegative = Config->IsNegativeHardwareChannel(iHardwChan);
    QString s = ( bNegative ? "Neg" : "Pos");
    ui->lePolar->setText(s);

    // Check is channel number is valid
    int max = (bUseLogical ? Config->CountLogicalChannels() : Reader->CountChannels());
    max--;
    if (val > max )
    {
        if (val == 0) return; //in case no channels are defined
        ui->sbChannel->setValue(max);
        return;  // will return to this cycle with on_changed signal
    }

    // Event/Signal check and indication
    const int numEvents = (bFromDataHub ? DataHub->CountEvents() : Extractor->CountEvents());
    if (numEvents == 0)
    {
        ui->leSignal->setText("");
        return;
    }
    if (ievent>numEvents)
    {
        ui->sbEvent->setValue(numEvents-1);
        return;
    }
    QString ss;
    if (bFromDataHub)
    {
        if (DataHub->IsRejected(ievent)) ss = "Rejected event";
        else
        {
            const int numChannels = DataHub->CountChannels();
            if ( val >= numChannels ) ss = "n.a.";  //paranoic :)
            else
            {
                double signal = DataHub->GetSignal(ievent, val);
                if ( std::isnan(signal) ) ss = "n.a.";
                else ss = QString::number(signal);
            }
        }
    }
    else
    {
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
    }
    ui->leSignal->setText(ss);    
}

void MainWindow::on_pbShowWaveform_toggled(bool checked)
{
#ifdef CERN_ROOT
    RootModule->ShowSingleWaveWindow(checked);
    LogMessage("");
    if (!checked) return;

    const bool bFromDataHub = (ui->cobExplorerSource->currentIndex() == 1);
    int numEvents = bFromDataHub ? DataHub->CountEvents() : Reader->CountEvents();
    int ievent = ui->sbEvent->value();
    if (ievent >= numEvents)
    {
        ui->sbEvent->setValue(0);
        RootModule->ClearSingleWaveWindow();
        return;
    }

    int ichannel;
    if (bFromDataHub)
    {
        ichannel = ui->sbChannel->value();
        if (ichannel >= DataHub->CountChannels())
        {
            RootModule->ClearSingleWaveWindow();
            return;
        }
    }
    else
    {
        ichannel = getCurrentlySelectedHardwareChannel();
        if (ichannel<0 || !Reader->isValid())
        {
            RootModule->ClearSingleWaveWindow();
            return;
        }
    }

    bool bNegative = bFromDataHub ? Config->IsNegativeLogicalChannel(ichannel) : Config->IsNegativeHardwareChannel(ichannel);
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

    bool bOK = RootModule->DrawSingle(bFromDataHub, ievent, ichannel, ui->cbAutoscaleY->isChecked(), Min, Max);
    if (!bOK) RootModule->ClearSingleWaveWindow();

#else
    QMessageBox::information(this, "", "Cern ROOT module was not configured!", QMessageBox::Ok, QMessageBox::Ok);
#endif
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

    const bool bFromDataHub = (ui->cobExplorerSource->currentIndex() == 1);
    int numEvents = bFromDataHub ? DataHub->CountEvents() : Reader->CountEvents();
    int ievent = ui->sbEvent->value();
    if (ievent >= numEvents)
    {
        ui->sbEvent->setValue(0);
        RootModule->ClearSingleWaveWindow();
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

    bool bOK = RootModule->DrawOverlay(bFromDataHub, ievent, bNeg, ui->cbAutoscaleY->isChecked(), Min, Max, ui->cobSortBy->currentIndex());
    if (!bOK)
    {
        if (bNeg) RootModule->ClearOverNegWaveWindow();
        else      RootModule->ClearOverPosWaveWindow();
    }

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

    const bool bFromDataHub = (ui->cobExplorerSource->currentIndex() == 1);
    int numEvents = bFromDataHub ? DataHub->CountEvents() : Reader->CountEvents();
    int ievent = ui->sbEvent->value();
    if (ievent >= numEvents)
    {
        ui->sbEvent->setValue(0);
        RootModule->ClearSingleWaveWindow();
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

    bool bOK = RootModule->DrawAll(bFromDataHub, ievent, bNeg, padsX, padsY, ui->cbAutoscaleY->isChecked(), Min, Max, ui->cobSortBy->currentIndex(), ui->cbLabels->isChecked());
    if (!bOK)
    {
        if (bNeg) RootModule->ClearAllNegWaveWindow();
        else      RootModule->ClearAllPosWaveWindow();
    }

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
    OnEventOrChannelChanged();
    on_sbEvent_valueChanged(ui->sbEvent->value());
}

void MainWindow::on_ledMaxNeg_editingFinished()
{
    OnEventOrChannelChanged();
    on_sbEvent_valueChanged(ui->sbEvent->value());
}

void MainWindow::on_ledMinPos_editingFinished()
{
    OnEventOrChannelChanged();
    on_sbEvent_valueChanged(ui->sbEvent->value());
}

void MainWindow::on_ledMaxPos_editingFinished()
{
    OnEventOrChannelChanged();
    on_sbEvent_valueChanged(ui->sbEvent->value());
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
    //setGeometry(10,10,600,800);
    this->move(10, 10); this->resize(600, 800);
    //ScriptWindow->setGeometry(670,10,600,800);
    ScriptWindow->move(670, 10); ScriptWindow->resize(600, 800);

#ifdef CERN_ROOT
    RootModule->ResetPositionOfWindows();
#endif
}

void MainWindow::on_cobSignalExtractionMethod_currentIndexChanged(int index)
{
    ui->sbExtractAllFromSampleNumber->setVisible(index == 2);
    ui->frIntegration->setVisible(index == 3);
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

const QString MainWindow::PackChannelList(QVector<int> vec)
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

void MainWindow::bulkProcessorEnvelope(const QStringList FileNames)
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
    numProcessedEvents = 0;
    numBadEvents = 0;
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

    LogMessage("Processed events: " + QString::number(numProcessedEvents) + "  Disreguarded events: " + QString::number(numBadEvents) );

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
    numProcessedEvents += Reader->CountAllProcessedEvents();
    numBadEvents += Reader->CountBadEvents();

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
    int numEvents = Extractor->CountEvents();
    int numChannels = Extractor->CountChannels();
    if (numEvents == 0 || numChannels == 0 || numEvents != Reader->CountEvents() || numChannels != Reader->CountChannels())
    {
        ui->pteBulkLog->appendPlainText("---- Extractor data not valid -> ignoring this file");
        return false;
    }
    if (!Config->Map->Validate().isEmpty())
    {
        qDebug() << Config->Map->Validate();
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
                QVector< QVector<float>* > vec;
                for (int ihardw : map)
                {
                    if (Extractor->GetSignalFast(iev, ihardw) == 0 || Config->IsIgnoredHardwareChannel(ihardw)) vec << 0;
                    else
                    {
                        QVector<float>* wave = new QVector<float>();
                        *wave = *Reader->GetWaveformPtrFast(iev, ihardw);
                        vec << wave;
                    }
                }
                ev->SetWaveforms(&vec);
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

    QString FileName = QFileDialog::getSaveFileName(this, "Save events from DataHub", Config->WorkingDir, "Data files (*.dat *.txt);;All files (*.*)");
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


    const bool bSavePositions = ui->cbAddReconstructedPositions->isChecked();
    const bool bSkipRejected = ui->cbSaveOnlyGood->isChecked();
    for (int iev=0; iev<numEvents; iev++)
    {
        if (bSkipRejected)
            if (DataHub->IsRejectedFast(iev)) continue;
        const QVector<float>* vec = DataHub->GetSignalsFast(iev);
        for (float val : *vec) outStream << QString::number(val) << " ";
        if (bSavePositions)
        {
            const float* R = DataHub->GetPositionFast(iev);
            outStream << "     " << R[0] << " " << R[1] << " " << R[2];
        }
        outStream << "\r\n";
    }
}

void MainWindow::on_cobExplorerSource_currentIndexChanged(int index)
{
    const bool bDirect = (index == 0);

    ui->frExploreDirectly->setVisible(bDirect);
    ui->cobHardwareOrLogical->setVisible(bDirect);
    ui->labLogicalChannelNumber->setVisible(!bDirect);
    ui->cobSortBy->setVisible(bDirect);

    OnEventOrChannelChanged();
    on_sbEvent_valueChanged(ui->sbEvent->value());
}

void MainWindow::updateNumEventsIndication()
{
    ui->labDatahubEvents->setText("DataHub contains " + QString::number(DataHub->CountEvents()) + " events");

    const bool bFromDataHub = (ui->cobExplorerSource->currentIndex()==1);
    const int numEvents = ( bFromDataHub ? DataHub->CountEvents() : Extractor->CountEvents());
    ui->leNumEvents->setText( QString::number(numEvents) );
}

void MainWindow::on_pbClearDataHub_clicked()
{
    DataHub->Clear();
    UpdateGui();
}

void MainWindow::on_pbLoadToDataHub_clicked()
{
    if ( DataHub->CountEvents() != 0 && !bNeverRemindAppendToHub)
    {
        QMessageBox mb;
        mb.setText("DataHub is not empty - data will be appended");
        mb.addButton("OK", QMessageBox::YesRole);
        QAbstractButton* ConfAlways = mb.addButton("OK, and do not remind", QMessageBox::YesRole);
        QAbstractButton* Nope = mb.addButton("Cancel", QMessageBox::NoRole);
        mb.setIcon(QMessageBox::Question);
        mb.exec();

        if (mb.clickedButton() == Nope) return;
        if (mb.clickedButton() == ConfAlways) bNeverRemindAppendToHub = true;
    }

    QString FileName = QFileDialog::getOpenFileName(this, "Load events", Config->WorkingDir, "Data files (*.dat *.txt);;All files (*.*)");
    if (FileName.isEmpty()) return;
    Config->WorkingDir = QFileInfo(FileName).absolutePath();

    QFile inFile( FileName );
    inFile.open(QIODevice::ReadOnly);
    if(!inFile.isOpen())
      {
        message("Unable to open file " +FileName+ " for reading!", this);
        return;
      }
    QTextStream inStream(&inFile);

    this->setEnabled(false);
    ui->prbMainBar->setVisible(true);

    int numChannels = Config->CountLogicalChannels();
    int upperLim = numChannels;
    bool bLoadXYZ = ui->cbLoadIncludeReconstructed->isChecked();
    if (bLoadXYZ) upperLim += 3;
    int numEvents = 0;
    qint64 totSize = QFileInfo(FileName).size();
    while (!inStream.atEnd())  // optimized assuming proper format of the file
    {
        const QString s = inStream.readLine();

        if (numEvents % 200 == 0)
        {
            ui->prbMainBar->setValue(100.0 * inStream.pos() / totSize);
            updateNumEventsIndication();
            qApp->processEvents();
        }

        QRegExp rx("(\\ |\\,|\\:|\\t)");
        QStringList fields = s.split(rx, QString::SkipEmptyParts);
        if (fields.size() < upperLim) continue;

        QVector<float>* vec = new QVector<float>(numChannels);
        for (int i=0; i<numChannels; i++)
        {
            bool bOK;
            const QString f = fields.at(i);
            float val = f.toFloat(&bOK);
            if (!bOK)
            {
                delete vec;
                continue;
            }
            (*vec)[i] = val;
        }

        float xyz[3];
        if (bLoadXYZ)
        {

            bool bOK;
            for (int i=0; i<3; i++)
            {
                const QString ss = fields.at(numChannels+i);
                xyz[i] = ss.toFloat(&bOK);
                if (!bOK)
                {
                    delete vec;
                    continue;
                }
            }
        }

        AOneEvent* ev = new AOneEvent();
        ev->SetSignals(vec);  // transfer ownership!
        if (bLoadXYZ) ev->SetPosition(xyz);

        DataHub->AddEvent(ev);
        numEvents++;
    }

    setEnabled(true);
    ui->prbMainBar->setVisible(false);
    message("Added " + QString::number(numEvents) + " events", this);
    UpdateGui();
}
