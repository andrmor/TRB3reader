#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "masterconfig.h"
#include "ajsontools.h"
#include "channelmapper.h"
#include "trb3signalextractor.h"
#include "trb3datareader.h"
#include "ascriptwindow.h"
#include "adispatcher.h"
#include "adatahub.h"
#include "amessage.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>

#ifdef CERN_ROOT
#include "cernrootmodule.h"
#endif

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveCompleteState();

#ifdef CERN_ROOT
    delete RootModule; RootModule = 0;
#endif

    QMainWindow::closeEvent(event);
}

void MainWindow::saveCompleteState()
{
    //save script-related config
    QJsonObject jsS;
    ScriptWindow->WriteToJson(jsS);
    SaveJsonToFile(jsS, Dispatcher->ConfigDir+"/scripting.json");

    Dispatcher->SaveConfig(Dispatcher->AutosaveFile);
}

void MainWindow::on_actionLoad_config_triggered()
{
    QString FileName = QFileDialog::getOpenFileName(this, "Load configuration", "", "Json files (*.json)");
    if (FileName.isEmpty()) return;

    Dispatcher->LoadConfig(FileName);
}

void MainWindow::on_actionSave_config_triggered()
{
    QString FileName = QFileDialog::getSaveFileName(this, "Save configuration", "", "Json files (*.json)");
    if (FileName.isEmpty()) return;
    if (QFileInfo(FileName).suffix().isEmpty()) FileName += ".json";

    Dispatcher->SaveConfig(FileName);
}

void MainWindow::WriteGUItoJson(QJsonObject &json)
{
    QJsonObject jsgui;

    jsgui["HardOrLog"] = ui->cobHardwareOrLogical->currentIndex();

    jsgui["KeepEventsOnStart"] = ui->cbKeepEvents->isChecked();
    jsgui["BulkExtract"] = ui->cbBulkExtract->isChecked();
    jsgui["AutoRunScript"] = ui->cbAutoExecuteScript->isChecked();
    jsgui["SaveFiles"] = ui->cbSaveSignalsToFiles->isChecked();
    jsgui["SuffixReplacement"] = ui->leAddToProcessed->text();
    jsgui["BulkCopy"] = ui->cbBulkCopyToDatahub->isChecked();
    jsgui["BulkCopyWaveforms"] = ui->cbBulkAlsoCopyWaveforms->isChecked();
    jsgui["SaveAddPositions"] = ui->cbAddReconstructedPositions->isChecked();
    jsgui["SaveSkipRejected"] = ui->cbSaveOnlyGood->isChecked();
    jsgui["LoadAlsoPositions"] = ui->cbLoadIncludeReconstructed->isChecked();

    jsgui["ExplorerSource"] = ui->cobExplorerSource->currentIndex();

    QJsonObject ja;
        ja["AutoY"] = ui->cbAutoscaleY->isChecked();
        ja["Sort"] = ui->cobSortBy->currentIndex();
        ja["MinPos"] = ui->ledMinPos->text().toDouble();
        ja["MinNeg"] = ui->ledMinNeg->text().toDouble();
        ja["MaxPos"] = ui->ledMaxPos->text().toDouble();
        ja["MaxNeg"] = ui->ledMaxNeg->text().toDouble();
    jsgui["GraphScale"] = ja;

    QJsonObject jx;
        jx["NegX"] = ui->sbAllNegX->value();
        jx["NegY"] = ui->sbAllNegY->value();
        jx["PosX"] = ui->sbAllPosX->value();
        jx["PosY"] = ui->sbAllPosY->value();
        jx["Labels"] = ui->cbLabels->isChecked();
        jx["LabelsType"] = ui->cobLableType->currentIndex();
    jsgui["AllGraph"] = jx;

    json["GUI"] = jsgui;
}

void MainWindow::ReadGUIfromJson(const QJsonObject& json)
{
    if (!json.contains("GUI")) return;

    QJsonObject jsgui = json["GUI"].toObject();

    JsonToComboBox(jsgui, "HardOrLog", ui->cobHardwareOrLogical);

    JsonToCheckbox(jsgui, "KeepEventsOnStart", ui->cbKeepEvents);
    JsonToCheckbox(jsgui, "BulkExtract", ui->cbBulkExtract);
    JsonToCheckbox(jsgui, "AutoRunScript", ui->cbAutoExecuteScript);
    JsonToCheckbox(jsgui, "SaveFiles", ui->cbSaveSignalsToFiles);
    JsonToLineEditText(jsgui, "SuffixReplacement", ui->leAddToProcessed);
    JsonToCheckbox(jsgui, "BulkCopy", ui->cbBulkCopyToDatahub);
    JsonToCheckbox(jsgui, "BulkCopyWaveforms", ui->cbBulkAlsoCopyWaveforms);
    JsonToCheckbox(jsgui, "SaveAddPositions", ui->cbAddReconstructedPositions);
    JsonToCheckbox(jsgui, "SaveSkipRejected", ui->cbSaveOnlyGood);
    JsonToCheckbox(jsgui, "LoadAlsoPositions", ui->cbLoadIncludeReconstructed);

    JsonToComboBox(jsgui, "ExplorerSource", ui->cobExplorerSource);

    QJsonObject ja = jsgui["GraphScale"].toObject();
        JsonToCheckbox(ja, "AutoY", ui->cbAutoscaleY);
        JsonToComboBox(ja, "Sort", ui->cobSortBy);
        JsonToLineEdit(ja, "MinPos", ui->ledMinPos);
        JsonToLineEdit(ja, "MinNeg", ui->ledMinNeg);
        JsonToLineEdit(ja, "MaxPos", ui->ledMaxPos);
        JsonToLineEdit(ja, "MaxNeg", ui->ledMaxNeg);

    QJsonObject jx;
    parseJson(jsgui, "AllGraph", jx);
    if (!jx.isEmpty())
    {
        JsonToSpinBox(jx, "NegX", ui->sbAllNegX);
        JsonToSpinBox(jx, "NegY", ui->sbAllNegY);
        JsonToSpinBox(jx, "PosX", ui->sbAllPosX);
        JsonToSpinBox(jx, "PosY", ui->sbAllPosY);
        JsonToCheckbox(jx, "Labels", ui->cbLabels);
        JsonToComboBox(jx, "LabelsType", ui->cobLableType);
    }
}

void MainWindow::SaveWindowSettings()
{
    QJsonObject json;

    json["Main"] = SaveWindowToJson(x(), y(), width(), height(), true);
    json["ScriptWindow"] = SaveWindowToJson(ScriptWindow->x(), ScriptWindow->y(), ScriptWindow->width(), ScriptWindow->height(), ScriptWindow->isVisible());

#ifdef CERN_ROOT
    json["GraphWindows"] = RootModule->SaveGraphWindows();
#endif

    SaveJsonToFile(json, Dispatcher->WinSetFile);
}

void MainWindow::LoadWindowSettings()
{
    QJsonObject js;
    LoadJsonFromFile(js, Dispatcher->WinSetFile);
    if (js.isEmpty()) return;

    int x=10, y=10, w=500, h=700;
    bool bVis=true;

    if (js.contains("Main"))
    {
        QJsonObject jsMain = js["Main"].toObject();
        LoadWindowFromJson(jsMain, x, y, w, h, bVis);
        this->move(x, y);
        this->resize(w, h);
        //setGeometry(x, y, w, h); // introduces a shift up on Windows7
    }

    if (js.contains("Main"))
    {
        QJsonObject jsScript = js["ScriptWindow"].toObject();
        LoadWindowFromJson(jsScript, x, y, w, h, bVis);
        ScriptWindow->move(x, y);
        ScriptWindow->resize(w, h);
        //ScriptWindow->setGeometry(x, y, w, h); // introduces a shift up on Windows7
        ScriptWindow->setVisible(bVis);
    }

#ifdef CERN_ROOT
    QJsonObject jsW;
    parseJson(js, "GraphWindows", jsW);
    RootModule->SetWindowGeometries(jsW);
#endif
}

// --- Update GUI controls on Config change ---
void MainWindow::UpdateGui()
{
    //qDebug() << "--- Updating GUI";

    //datakinds
    ui->lwDatakinds->clear();
    QVector<int> datakinds = Config->GetListOfDatakinds();
    if ( datakinds.size() > 1 ) std::sort(datakinds.begin(), datakinds.end());
    for (int i : datakinds)
    {
        QString s = QString::number(i) + "   (hex: " + QString::number(i, 16) + ")";
        QListWidgetItem* item = new QListWidgetItem(s);
        item->setTextAlignment(Qt::AlignCenter);
        ui->lwDatakinds->addItem(item);
    }

    ui->leFileName->setText(Config->FileName);

    ui->ptePolarity->clear();    
    //for (int i: Config->GetListOfNegativeChannels()) s += QString::number(i)+" ";
    QString s = PackChannelList(Config->GetListOfNegativeChannels());
    ui->ptePolarity->appendPlainText(s);

    ui->pteMapping->clear();
    s.clear();
    for (int i: Config->GetMapping()) s += QString::number(i)+" ";
    ui->pteMapping->appendPlainText(s);

    ui->pteIgnoreHardwareChannels->clear();
//    s.clear();
//    std::vector<int> ign;
//    for (int i: Config->IgnoreHardwareChannels) ign.push_back(i);
//    std::sort(ign.begin(), ign.end());
//    for (int i: ign) s += QString::number(i)+" ";
    s = PackChannelList(Config->GetListOfIgnoreChannels());
    ui->pteIgnoreHardwareChannels->appendPlainText(s);

    ui->cbSubstractPedestal->setChecked(Config->bPedestalSubstraction);
        ui->sbPedestalFrom->setValue(Config->PedestalFrom);
        ui->sbPedestalTo->setValue(Config->PedestalTo);

    ui->cbSmoothWaveforms->setChecked(Config->bSmoothWaveforms);
    ui->cbSmoothBeforePedestal->setChecked(Config->bSmoothingBeforePedestals);
        ui->cbAdjacentAveraging->setChecked(Config->AdjacentAveraging_bOn);
            ui->sbAdjAvPoints->setValue(Config->AdjacentAveraging_NumPoints);
            ui->cbAdjAvWeighted->setChecked(Config->AdjacentAveraging_bWeighted);

    int method = Config->SignalExtractionMethod;
    if (method <= 3) ui->cobSignalExtractionMethod->setCurrentIndex(method);
    else
        message("Signal extraction method in config file is not valid in this version of program", this);


    ui->sbExtractAllFromSampleNumber->setValue(Config->CommonSampleNumber);
    ui->sbIntegrateFrom->setValue(Config->IntegrateFrom);
    ui->sbIntegrateTo->setValue(Config->IntegrateTo);

    ui->cbZeroSignalIfReverseMax->setChecked(Config->bZeroSignalIfReverse);
        ui->ledReverseMaxLimit->setText(QString::number(Config->ReverseMaxThreshold));

    ui->cbPosThreshold->setChecked(Config->bPositiveThreshold);
    ui->ledPosThresholdMin->setText(QString::number(Config->PositiveThreshold));

    ui->cbNegThreshold->setChecked(Config->bNegativeThreshold);
    ui->ledNegThresholdMin->setText(QString::number(Config->NegativeThreshold));

    ui->cbIgnorePosThreshold->setChecked(Config->bPositiveIgnore);
    ui->ledPosIgnoreMax->setText(QString::number(Config->PositiveIgnore));

    ui->cbIgnoreNegThreshold->setChecked(Config->bNegativeIgnore);
    ui->ledNegIgnoreMax->setText(QString::number(Config->NegativeIgnore));

    ui->cbNegMaxSignalGate->setChecked(Config->bNegMaxGate);
    ui->sbNegMaxFrom->setValue(Config->NegMaxGateFrom);
    ui->sbNegMaxTo->setValue(Config->NegMaxGateTo);
    ui->cbPosMaxSignalGate->setChecked(Config->bPosMaxGate);
    ui->sbPosMaxFrom->setValue(Config->PosMaxGateFrom);
    ui->sbPosMaxTo->setValue(Config->PosMaxGateTo);

    updateNumEventsIndication();
    OnEventOrChannelChanged();
}

// --- update Config on GUI operated by user ---
void MainWindow::on_cbSubstractPedestal_clicked(bool checked)
{
    Config->bPedestalSubstraction = checked;
    ClearData();
}

void MainWindow::on_sbPedestalFrom_editingFinished()
{
    Config->PedestalFrom = ui->sbPedestalFrom->value();
    ClearData();
}

void MainWindow::on_sbPedestalTo_editingFinished()
{
    Config->PedestalTo = ui->sbPedestalTo->value();
    ClearData();
}

void MainWindow::on_cbSmoothWaveforms_clicked(bool checked)
{
    Config->bSmoothWaveforms = checked;
    ClearData();
}

void MainWindow::on_cbSmoothBeforePedestal_clicked(bool checked)
{
    Config->bSmoothingBeforePedestals = checked;
    ClearData();
}

void MainWindow::on_cbAdjacentAveraging_clicked(bool checked)
{
    Config->AdjacentAveraging_bOn = checked;
    ClearData();
}

void MainWindow::on_sbAdjAvPoints_editingFinished()
{
    Config->AdjacentAveraging_NumPoints = ui->sbAdjAvPoints->value();
    ClearData();
}

void MainWindow::on_cbAdjAvWeighted_clicked(bool checked)
{
    Config->AdjacentAveraging_bWeighted = checked;
    ClearData();
}

void MainWindow::on_cobSignalExtractionMethod_activated(int index)
{
    Config->SignalExtractionMethod = index;
    ClearData();
}

void MainWindow::on_cbZeroSignalIfReverseMax_clicked(bool checked)
{
    Config->bZeroSignalIfReverse = checked;
    ClearData();
}

void MainWindow::on_ledReverseMaxLimit_editingFinished()
{
    Config->ReverseMaxThreshold = ui->ledReverseMaxLimit->text().toDouble();
    ClearData();
}

void MainWindow::on_cbPosThreshold_clicked(bool checked)
{
    Config->bPositiveThreshold = checked;
    ClearData();
}

void MainWindow::on_cbNegThreshold_clicked(bool checked)
{
    Config->bNegativeThreshold = checked;
    ClearData();
}

void MainWindow::on_ledPosThresholdMin_editingFinished()
{
    Config->PositiveThreshold = ui->ledPosThresholdMin->text().toDouble();
    ClearData();
}

void MainWindow::on_ledNegThresholdMin_editingFinished()
{
    Config->NegativeThreshold = ui->ledNegThresholdMin->text().toDouble();
    ClearData();
}

void MainWindow::on_cbIgnorePosThreshold_clicked(bool checked)
{
    Config->bPositiveIgnore = checked;
    ClearData();
}

void MainWindow::on_cbIgnoreNegThreshold_clicked(bool checked)
{
    Config->bNegativeIgnore = checked;
    ClearData();
}

void MainWindow::on_ledPosIgnoreMax_editingFinished()
{
    Config->PositiveIgnore = ui->ledPosIgnoreMax->text().toDouble();
    ClearData();
}

void MainWindow::on_ledNegIgnoreMax_editingFinished()
{
    Config->NegativeIgnore = ui->ledNegIgnoreMax->text().toDouble();
    ClearData();
}

void MainWindow::on_cbPosMaxSignalGate_clicked(bool checked)
{
    Config->bPosMaxGate = checked;
    ClearData();
}

void MainWindow::on_cbNegMaxSignalGate_clicked(bool checked)
{
    Config->bNegMaxGate = checked;
    ClearData();
}

void MainWindow::on_sbPosMaxFrom_editingFinished()
{
    Config->PosMaxGateFrom = ui->sbPosMaxFrom->value();
    ClearData();
}

void MainWindow::on_sbPosMaxTo_editingFinished()
{
    Config->PosMaxGateTo = ui->sbPosMaxTo->value();
    ClearData();
}

void MainWindow::on_sbNegMaxFrom_editingFinished()
{
    Config->NegMaxGateFrom = ui->sbNegMaxFrom->value();
    ClearData();
}

void MainWindow::on_sbNegMaxTo_editingFinished()
{
    Config->NegMaxGateTo = ui->sbNegMaxTo->value();
    ClearData();
}

void MainWindow::on_sbExtractAllFromSampleNumber_editingFinished()
{
    Config->CommonSampleNumber = ui->sbExtractAllFromSampleNumber->value();
    ClearData();
}

void MainWindow::on_sbIntegrateFrom_editingFinished()
{
    Config->IntegrateFrom = ui->sbIntegrateFrom->value();
    ClearData();
}

void MainWindow::on_sbIntegrateTo_editingFinished()
{
    Config->IntegrateTo = ui->sbIntegrateTo->value();
    ClearData();
}

