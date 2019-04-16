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
#include "aservermonitorwindow.h"
#include "atrbruncontrol.h"

#ifdef CERN_ROOT
#include "cernrootmodule.h"
#endif

#include "trb3datareader.h"
#include "trb3signalextractor.h"
#include "ahldfileprocessor.h"
#include "anetworkmodule.h"

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>
#include <QInputDialog>
#include <QProgressBar>
#include <QTimer>
#include <QElapsedTimer>

#include <cmath>

MainWindow::MainWindow(MasterConfig* Config,
                       ADispatcher *Dispatcher,
                       ADataHub* DataHub,
                       Trb3dataReader* Reader,
                       Trb3signalExtractor* Extractor,
                       AHldFileProcessor& HldFileProcessor, ANetworkModule &Network,
                       QWidget *parent) :
    QMainWindow(parent),
    Config(Config), Dispatcher(Dispatcher), DataHub(DataHub), Reader(Reader), Extractor(Extractor), HldFileProcessor(HldFileProcessor), Network(Network),
    ui(new Ui::MainWindow)
{
    bStopFlag = false;
    ui->setupUi(this);

    TrbRunManager = new ATrbRunControl(*Config, Network, Dispatcher->ConfigDir);
    QObject::connect(TrbRunManager, &ATrbRunControl::sigBoardIsAlive, this, &MainWindow::onBoardIsAlive);
    QObject::connect(TrbRunManager, &ATrbRunControl::sigBoardOff, this, &MainWindow::onBoardDisconnected);
    QObject::connect(TrbRunManager, &ATrbRunControl::boardLogReady, this, &MainWindow::onBoardLogNewText);
    QObject::connect(TrbRunManager, &ATrbRunControl::sigAcquireIsAlive, this, &MainWindow::onAcquireIsAlive);

    watchdogTimer = new QTimer();
    watchdogTimer->setInterval(2000);
    QObject::connect(watchdogTimer, &QTimer::timeout, this, &MainWindow::onWatchdogFailed);

    aTimer = new QTimer();
    aTimer->setSingleShot(true);
    QObject::connect(aTimer, &QTimer::timeout, this, &MainWindow::on_pbStopAcquire_clicked);

    elTimer = new QElapsedTimer();

    QDoubleValidator* dv = new QDoubleValidator(this);
    dv->setNotation(QDoubleValidator::ScientificNotation);
    QList<QLineEdit*> list = this->findChildren<QLineEdit *>();
    foreach(QLineEdit *w, list) if (w->objectName().startsWith("led")) w->setValidator(dv);

    ui->pbUpdateTriggerSettings->setVisible(false);
    ui->pbRefreshBufferIndication->setVisible(false);
    ui->pbUpdateTriggerGui->setVisible(false);

#ifdef CERN_ROOT
    RootModule = new CernRootModule(Reader, Extractor, Config, DataHub);
    connect(RootModule, &CernRootModule::WOneHidden, [=](){ui->pbShowWaveform->setChecked(false);});
    connect(RootModule, &CernRootModule::WOverNegHidden, [=](){ui->pbShowOverlayNeg->setChecked(false);});
    connect(RootModule, &CernRootModule::WOverPosHidden, [=](){ui->pbShowOverlayPos->setChecked(false);});
    connect(RootModule, &CernRootModule::WAllNegHidden, [=](){ui->pbShowAllNeg->setChecked(false);});
    connect(RootModule, &CernRootModule::WAllPosHidden, [=](){ui->pbShowAllPos->setChecked(false);});
#else
    qDebug() << "-> Graph module (based on CERN ROOT) was NOT compiled";
#endif

    connect(DataHub, &ADataHub::requestGuiUpdate, this, &MainWindow::UpdateGui);
    connect(DataHub, &ADataHub::reportProgress, this, &MainWindow::onProgressUpdate);

    //messaging during bulk processing of hld files
    connect(&HldFileProcessor, &AHldFileProcessor::LogMessage, this, &MainWindow::onShowMessageRequest);
    connect(&HldFileProcessor, &AHldFileProcessor::LogAction, this, &MainWindow::onShowActionRequest);

    //Creating script window, registering script units, and setting up QObject connections
    CreateScriptWindow();
    connect(&HldFileProcessor, &AHldFileProcessor::RequestExecuteScript, ScriptWindow, &AScriptWindow::ExecuteScriptInFirstTab);

    //Loading window settings
    LoadWindowSettings();
    QJsonObject jsS;
    LoadJsonFromFile(jsS, Dispatcher->ConfigDir+"/scripting.json");
    if (!jsS.isEmpty()) ScriptWindow->ReadFromJson(jsS);

    //misc gui settings
    menuBar()->setNativeMenuBar(false);  //otherwise on some system menu bar is not wisible!
    ui->prbMainBar->setVisible(false);
    ui->cbAutoscaleY->setChecked(true);
    ui->pbStop->setVisible(false);
    on_cobSignalExtractionMethod_currentIndexChanged(ui->cobSignalExtractionMethod->currentIndex());
    OnEventOrChannelChanged(); // to update channel mapping indication

    ServerWindow = new AServerMonitorWindow(*this, Network, this);
    QObject::connect(&Network, &ANetworkModule::StatusChanged, ServerWindow, &AServerMonitorWindow::onServerstatusChanged);
    QObject::connect(&Network, &ANetworkModule::ReportTextToGUI, ServerWindow, &AServerMonitorWindow::appendText);
}

MainWindow::~MainWindow()
{
#ifdef CERN_ROOT
    delete RootModule;
#endif

    delete watchdogTimer;
    delete aTimer;
    delete elTimer;

    delete TrbRunManager;
    delete ScriptWindow;
    delete ui;
}

void MainWindow::SetEnabled(bool flag)
{
    ui->twMain->setEnabled(flag);
    menuBar()->setEnabled(flag);

    ScriptWindow->setEnabled(flag);
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
    QString err = Reader->Read(Config->FileName);
    if (!err.isEmpty()) return err;

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
        if (ichannel<0 || Reader->isEmpty())
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

    bool bOK = RootModule->DrawAll(bFromDataHub, ievent, bNeg, padsX, padsY,
                                   ui->cbAutoscaleY->isChecked(), Min, Max,
                                   ui->cobSortBy->currentIndex(),
                                   ui->cbLabels->isChecked(), ui->cobLableType->currentIndex());
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
    //int datakind = QInputDialog::getInt(this, "TRBreader", "Input new datakind to add", 0, 0, 0xFFFF, 1, &bOK);
    QString datakindStr = QInputDialog::getText(this, "TRBreader", "Input new address (start with 0x for hexadecimal)", QLineEdit::Normal,
                                             QString(), &bOK);
    int datakind = 0;
    if (datakindStr.startsWith("0x"))
        datakind = datakindStr.toInt(&bOK, 16);
    else
        datakind = datakindStr.toInt(&bOK, 10);

    if (bOK)
        Config->AddDatakind(datakind);
    UpdateGui();
}

void MainWindow::on_pbRemoveDatakind_clicked()
{
    int raw = ui->lwDatakinds->currentRow();
    if (raw < 0)
    {
        message("Select datakind in th elist to remove by left-clicking on it", this);
        return;
    }
    QString sel = ui->lwDatakinds->currentItem()->text();
    QStringList sl = sel.split(" ");
    if (sl.size()>1)
    {
        QString dk = sl.first();
        int datakind = dk.toInt(0, 16);
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
    /*
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
    */

    int numProcessedEvents = 0;
    int numBadEvents = 0;
    for (QString name : FileNames)
    {
        bool bOK = HldFileProcessor.ProcessFile(name);
        if (!bOK) numErrors++;

        updateNumEventsIndication();
        qApp->processEvents();
        if (bStopFlag) break;
    }

    if (numErrors > 0) ui->pteBulkLog->appendPlainText("============= There were errors! =============");
    else ui->pteBulkLog->appendPlainText("Processed " + QString::number(FileNames.size()) + " files - no errors");

    LogMessage("Processed events: " + QString::number(numProcessedEvents) + "  Disreguarded events: " + QString::number(numBadEvents) );

    ui->twMain->setEnabled(true);
    ui->pbStop->setVisible(false);
    ui->pbStop->setChecked(false);

    UpdateGui();
}

void MainWindow::onShowMessageRequest(const QString message)
{
    ui->pteBulkLog->appendPlainText(message);
}

void MainWindow::onShowActionRequest(const QString action)
{
    LogMessage(action);
}

/*
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
    bool ok = Reader->Read(Config->FileName);
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
*/

void MainWindow::on_pbSaveSignalsFromDataHub_clicked()
{
    QString FileName = QFileDialog::getSaveFileName(this, "Save events from DataHub", Config->WorkingDir, "Data files (*.dat *.txt);;All files (*.*)");
    if (FileName.isEmpty()) return;
    Config->WorkingDir = QFileInfo(FileName).absolutePath();

    bool bSavePositions = ui->cbAddReconstructedPositions->isChecked();
    bool bSkipRejected = ui->cbSaveOnlyGood->isChecked();
    const QString ErrStr = DataHub->Save(FileName, bSavePositions, bSkipRejected);

    if(!ErrStr.isEmpty()) message(ErrStr, this);
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

void MainWindow::onBoardLogNewText(const QString text)
{
    QString txt = text;
    if (txt.endsWith('\n')) txt.chop(1);

    const QString warning = "WARNING: Could not get all ADCs to work despite retrying...";
    if (txt.contains(warning))
    {
        txt.remove(warning);
        txt += "\n<font color='red'>" + warning + "</font>";
        ui->pteBoardLog->appendHtml(txt);
    }
    else ui->pteBoardLog->appendPlainText(txt);

    if (txt.contains("Your board should be working now..."))
        ui->pteBoardLog->appendPlainText("Waiting for ready status...");

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

        DataHub->AddEventFast(ev);
        numEvents++;
    }

    setEnabled(true);
    ui->prbMainBar->setVisible(false);
    message("Added " + QString::number(numEvents) + " events", this);
    UpdateGui();
}

void MainWindow::on_cobLableType_activated(int)
{
    on_pbShowAllNeg_toggled(ui->pbShowAllNeg->isChecked());
    on_pbShowAllPos_toggled(ui->pbShowAllPos->isChecked());
}

void MainWindow::onProgressUpdate(int progress)
{
    ui->prbMainBar->setValue(progress);
}

void MainWindow::on_actionConfigure_WebSocket_server_triggered()
{
    ServerWindow->show();
}

//https://www.ssh.com/ssh/keygen/
    /*

    QStringList commands;
    //commands << "-hold";
    commands << "-iconic";
    //commands << "-C";
    commands << "-e";
    //QString s = QString("ssh %1@%2 'mkdir /home/rpcuser/test_dir'").arg(user).arg(host);
    QString s = QString("ssh %1@%2 '%3'").arg(user).arg(host).arg(start_script);
    //commands << "ssh username@host 'cd /home/user/backups; mysqldump -u root -p mydb > mydb.sql; echo DONE!'";
    commands << s;

    QProcess *process = new QProcess(0);
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start("xterm", commands);

    if(!process->waitForStarted()){
        qDebug() << "Could not wait to start...";
    }

    if(!process->waitForFinished()) {
        qDebug() << "Could not wait to finish...";
    }

    process->closeWriteChannel();
    qDebug() << process->readAll();

    */

void MainWindow::on_pbBoardOn_clicked()
{
    ui->labConnectionStatus->setText("<font color='orange'>Connecting</font>");

    QString err = TrbRunManager->StartBoard();
    if (!err.isEmpty())
        message(err, this);
    else
    {
        ui->leHost->setEnabled(false);
        ui->leUser->setEnabled(false);
    }
}

void MainWindow::on_pbBoardOff_clicked()
{
    watchdogTimer->stop();
    TrbRunManager->StopBoard();
    ui->pteBoardLog->appendPlainText("Disconnecting...");
}

void MainWindow::on_pbStartAcquire_clicked()
{
    int time_ms = -1;
    if (ui->cbLimitedTime->isChecked())
    {
        double sec = ui->ledTimeSpan->text().toDouble();

        int multiplier = 0;
        switch (ui->cobTimeUnits->currentIndex())
        {
        case 0:
            multiplier = 1;
            break;
        case 1:
            multiplier = 60;
            break;
        case 2:
            multiplier = 60*60;
            break;
        default:;
        }
        time_ms = sec * multiplier * 1000;
    }

    bLimitMaxEvents = ui->cbLimitEvents->isChecked();
    MaxEventsToRun = ui->leiMaxEvents->text().toInt();

    QString err = TrbRunManager->StartAcquire();
    if (err.isEmpty())
    {
        elTimer->start();
        if (ui->cbLimitedTime->isChecked()) aTimer->start(time_ms);
    }
    else message(err, this);

}

void MainWindow::on_pbStopAcquire_clicked()
{
    TrbRunManager->StopAcquire();
}

void MainWindow::onBoardIsAlive(double currentAccepetedRate)
{
    ui->labConnectionStatus->setText("<font color='green'>Connected</font>");
    if (currentAccepetedRate > 0) ui->leCurrentAceptedRate->setText(QString::number(currentAccepetedRate));
    watchdogTimer->start();
}

void MainWindow::onBoardDisconnected()
{
    ui->pteBoardLog->clear();
    ui->labConnectionStatus->setText("Not connected");
    ui->leCurrentAceptedRate->setText("");

    ui->leHost->setEnabled(true);
    ui->leUser->setEnabled(true);
}

void MainWindow::onAcquireIsAlive()
{
    int iSec = elTimer->elapsed()*0.001;
    ui->leStatTime->setText( QString::number(iSec) );
    ui->leStatNumEv->setText( QString::number(TrbRunManager->StatEvents) );
    ui->leStatData->setText( QString::number(TrbRunManager->StatData, 'g', 4) );
    ui->leStatRate->setText( QString::number(TrbRunManager->StatRate, 'g', 4) );
    ui->labDataUnits->setText( TrbRunManager->StatDataUnits );

    if (bLimitMaxEvents && TrbRunManager->StatEvents >= MaxEventsToRun)
        on_pbStopAcquire_clicked();
}

void MainWindow::onWatchdogFailed()
{
    ui->labConnectionStatus->setText("<font color='red'>Not responding</font>");
}

#include <QDesktopServices>
void MainWindow::on_pbOpenCTS_clicked()
{
    QDesktopServices::openUrl( QString("http://%1:1234/cts/cts.htm").arg(TrbRunManager->Host) );
}

void MainWindow::on_pbOpenBufferControl_clicked()
{
    QDesktopServices::openUrl( QString("http://%1:1234/addons/adc.pl?BufferConfig").arg(TrbRunManager->Host));
}

void MainWindow::on_leUser_editingFinished()
{
    Config->TrbRunSettings.User = ui->leUser->text();
}

void MainWindow::on_leHost_editingFinished()
{
    Config->TrbRunSettings.Host = ui->leHost->text();
}

void MainWindow::on_leDirOnHost_editingFinished()
{
    Config->TrbRunSettings.ScriptDirOnHost = ui->leDirOnHost->text();
}

void MainWindow::on_leStartupScriptOnHost_editingFinished()
{
    Config->TrbRunSettings.StartupScriptOnHost = ui->leStartupScriptOnHost->text();
}

void MainWindow::on_leStorageXmlOnHost_editingFinished()
{
    Config->TrbRunSettings.StorageXML = ui->leStorageXmlOnHost->text();
}

void MainWindow::on_leFolderForHldFiles_editingFinished()
{
    Config->TrbRunSettings.HldDirOnHost = ui->leFolderForHldFiles->text();
}

void MainWindow::on_leiHldFileSize_editingFinished()
{
    Config->TrbRunSettings.MaxHldSizeMb = ui->leiHldFileSize->text().toInt();
}

void MainWindow::on_ledTimeSpan_editingFinished()
{
    Config->TrbRunSettings.TimeLimit = ui->ledTimeSpan->text().toDouble();
}

void MainWindow::on_cobTimeUnits_activated(int index)
{
         if (index == 1)  Config->TrbRunSettings.TimeMultiplier = 60;
    else if (index == 2)  Config->TrbRunSettings.TimeMultiplier = 60*60;
    else                  Config->TrbRunSettings.TimeMultiplier = 1;
}

void MainWindow::on_cbLimitedTime_clicked(bool checked)
{
    Config->TrbRunSettings.bLimitTime = checked;
    //if (checked) ui->cbLimitEvents->setChecked(false);
}

void MainWindow::on_cbLimitEvents_clicked(bool checked)
{
    Config->TrbRunSettings.bLimitEvents = checked;
    //if (checked) ui->cbLimitedTime->setChecked(false);
}
void MainWindow::on_leiMaxEvents_editingFinished()
{
    Config->TrbRunSettings.MaxEvents = ui->leiMaxEvents->text().toInt();
}

void MainWindow::on_pbReadTriggerSettingsFromTrb_clicked()
{
    QString err = TrbRunManager->ReadTriggerSettingsFromBoard();
    if (!err.isEmpty()) message(err, this);

    on_pbUpdateTriggerGui_clicked();
}

void MainWindow::on_pbUpdateStartup_clicked()
{
    QString err = TrbRunManager->updateCTSsetupScript();
    if (err.isEmpty())
        message("Done!", this);
    else
        message(err, this);
}

void MainWindow::on_pbOpenCtsWebPage_clicked()
{
    QDesktopServices::openUrl( QString("http://%1:1234/cts/cts.htm").arg(TrbRunManager->Host) );
}

void MainWindow::on_pbSendCTStoTRB_clicked()
{
    this->SetEnabled(false);
    qApp->processEvents();

    QString err = TrbRunManager->sendCTStoTRB();

    this->SetEnabled(true);
    if (!err.isEmpty())
        message(err, this);
}

#include "abufferdelegate.h"
#include <QListWidgetItem>
#include <QListWidget>
void MainWindow::on_pbRefreshBufferIndication_clicked()
{
    ui->lwBufferControl->clear();

    const QVector<ABufferRecord> & recs = Config->getBufferRecords();
    for (const ABufferRecord & r : recs)
    {
        QListWidgetItem * item = new QListWidgetItem();
        ui->lwBufferControl->addItem(item);
        ABufferDelegate * wid = new ABufferDelegate();
        QObject::connect(wid, &ABufferDelegate::contentChanged, this, &MainWindow::onBufferDeleagateChanged);
        wid->setValues(r.Datakind, r.Samples, r.Delay, r.Downsampling);
        ui->lwBufferControl->setItemWidget(item, wid);
        item->setSizeHint( wid->sizeHint());
    }
}

void MainWindow::on_cbBufferSameValues_clicked(bool checked)
{

}

void MainWindow::on_cbBufferReadFromTRB_clicked()
{
    this->SetEnabled(false);
    qApp->processEvents();

    QString err = TrbRunManager->readBufferControlFromTRB();

    this->SetEnabled(true);
    if (!err.isEmpty())
        message(err, this);

    on_pbRefreshBufferIndication_clicked();
}

void MainWindow::on_pbBufferSendToTRB_clicked()
{
    this->SetEnabled(false);
    qApp->processEvents();

    QString err = TrbRunManager->sendBufferControlToTRB();

    this->SetEnabled(true);
    if (!err.isEmpty())
        message(err, this);
}

void MainWindow::on_pbBufferUpdateScript_clicked()
{
    this->SetEnabled(false);
    qApp->processEvents();

    QString err = TrbRunManager->updateBufferSetupScript();

    this->SetEnabled(true);
    if (!err.isEmpty())
        message(err, this);
}

void MainWindow::onBufferDeleagateChanged(ABufferDelegate * del)
{
    int addr, samples, delay, down;
    del->getValues(addr, samples, delay, down);

    ABufferRecord * rec = Config->findBufferRecord(addr);
    if (!rec)
    {
        qWarning() << "Error in find buffer record!";
        return;
    }

    bool bChanged = rec->updateValues(samples, delay, down);

    // TODO if common

    if (!bChanged) return; //no change in values

    // todo value canged -> update board? or just flag
}

void MainWindow::on_pbRestartTrb_clicked()
{
    if (!TrbRunManager->isBoardDisconnected())
        if (!areYouSure("The board is connected or connection procedure is in progress.\nRestart TRB?", this)) return;

    TrbRunManager->RestartBoard();
}

void MainWindow::on_pbUpdateTriggerGui_clicked()
{
    ui->cbMP0->setChecked(Config->TrbRunSettings.bMP_0);
    ui->cbMP1->setChecked(Config->TrbRunSettings.bMP_1);
    ui->cbMP2->setChecked(Config->TrbRunSettings.bMP_2);
    ui->cbMP3->setChecked(Config->TrbRunSettings.bMP_3);
    ui->cbMP4->setChecked(Config->TrbRunSettings.bMP_4);
    ui->cbMP5->setChecked(Config->TrbRunSettings.bMP_5);
    ui->cbMP6->setChecked(Config->TrbRunSettings.bMP_6);
    ui->cbMP7->setChecked(Config->TrbRunSettings.bMP_7);

    ui->cbRandomPulser->setChecked(Config->TrbRunSettings.bRandPulser);
    ui->cbPeriodicalPulser0->setChecked(Config->TrbRunSettings.bPeriodicPulser0);
    ui->cbPeriodicalPulser1->setChecked(Config->TrbRunSettings.bPeriodicPulser1);

    ulong rFreq = Config->TrbRunSettings.RandomPulserFrequency.toULong(nullptr, 16);
    double freq = (double)rFreq / 21.474836;
    ui->leRandomFrequency->setText( QString::number(freq) );

    ulong rPeriod = Config->TrbRunSettings.Period0.toULong(nullptr, 16);
    double per = (double)rPeriod * 10.0;
    ui->lePeriod0->setText( QString::number(per) );

    rPeriod = Config->TrbRunSettings.Period1.toULong(nullptr, 16);
    per = (double)rPeriod * 10.0;
    ui->lePeriod1->setText( QString::number(per) );
}

void MainWindow::on_pbUpdateTriggerSettings_clicked()
{
    Config->TrbRunSettings.bMP_0 = ui->cbMP0->isChecked();
    Config->TrbRunSettings.bMP_1 = ui->cbMP1->isChecked();
    Config->TrbRunSettings.bMP_2 = ui->cbMP2->isChecked();
    Config->TrbRunSettings.bMP_3 = ui->cbMP3->isChecked();
    Config->TrbRunSettings.bMP_4 = ui->cbMP4->isChecked();
    Config->TrbRunSettings.bMP_5 = ui->cbMP5->isChecked();
    Config->TrbRunSettings.bMP_6 = ui->cbMP6->isChecked();
    Config->TrbRunSettings.bMP_7 = ui->cbMP7->isChecked();

    Config->TrbRunSettings.bRandPulser = ui->cbRandomPulser->isChecked();
    Config->TrbRunSettings.bPeriodicPulser0 = ui->cbPeriodicalPulser0->isChecked();
    Config->TrbRunSettings.bPeriodicPulser1 = ui->cbPeriodicalPulser1->isChecked();

    double freq = ui->leRandomFrequency->text().toDouble() * 21.474836;
    ulong rFreq = (ulong)freq;
    if (rFreq > 0xffffffff) rFreq = 0xffffffff;
    Config->TrbRunSettings.RandomPulserFrequency = "0x" + QString::number(rFreq, 16);

    double per = 0.1 * ui->lePeriod0->text().toDouble();
    ulong rPer = (ulong)per;
    if (rPer > 0xffffffff) rPer = 0xffffffff;
    Config->TrbRunSettings.Period0 = "0x" + QString::number(rPer, 16);

    per = 0.1 * ui->lePeriod1->text().toDouble();
    rPer = (ulong)per;
    if (rPer > 0xffffffff) rPer = 0xffffffff;
    Config->TrbRunSettings.Period1 = "0x" + QString::number(rPer, 16);

    qDebug() <<Config->TrbRunSettings.RandomPulserFrequency<<Config->TrbRunSettings.Period0<<Config->TrbRunSettings.Period1;
}

void MainWindow::on_pbOpenBufferWebPage_clicked()
{
    on_pbOpenBufferControl_clicked();
}
