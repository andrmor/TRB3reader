#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "masterconfig.h"
#include "ajsontools.h"
#include "channelmapper.h"
#include "trb3signalextractor.h"
#include "trb3datareader.h"
#include "ascriptwindow.h"
#include "adispatcher.h"

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
    QJsonObject js;
#ifdef CERN_ROOT
    js = RootModule->SaveGraphWindows();
#endif
    Dispatcher->SaveConfig(Dispatcher->AutosaveFile, js);

    //save script-related config
    QJsonObject jsS;
    ScriptWindow->WriteToJson(jsS);
    SaveJsonToFile(jsS, Dispatcher->ConfigDir+"/scripting.json");
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

    QJsonObject jsW;
#ifdef CERN_ROOT
    jsW = RootModule->SaveGraphWindows();
#endif

    Dispatcher->SaveConfig(FileName, jsW);
}

void MainWindow::writeGUItoJson(QJsonObject &json)
{
    QJsonObject jsgui;

    jsgui["HardOrLog"] = ui->cobHardwareOrLogical->currentIndex();

    jsgui["BulkDir"] = ui->leDirForBulk->text();
    jsgui["AutoRunScript"] = ui->cbAutoExecuteScript->isChecked();
    jsgui["SaveFiles"] = ui->cbSaveSignalsToFiles->isChecked();

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
    jsgui["AllGraph"] = jx;

    json["GUI"] = jsgui;
}

void MainWindow::readGUIfromJson(QJsonObject &json)
{
    if (!json.contains("GUI")) return;

    QJsonObject jsgui = json["GUI"].toObject();

    JsonToComboBox(jsgui, "HardOrLog", ui->cobHardwareOrLogical);

    JsonToLineEditText(jsgui, "BulkDir", ui->leDirForBulk);
    JsonToCheckbox(jsgui, "AutoRunScript", ui->cbAutoExecuteScript);
    JsonToCheckbox(jsgui, "SaveFiles", ui->cbSaveSignalsToFiles);

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
    }
}

void MainWindow::writeWindowsToJson(QJsonObject& json, const QJsonObject jsW)
{
    QJsonObject js;

    js["Main"] = SaveWindowToJson(x(), y(), width(), height(), true);
    js["ScriptWindow"] = SaveWindowToJson(ScriptWindow->x(), ScriptWindow->y(), ScriptWindow->width(), ScriptWindow->height(), ScriptWindow->isVisible());
    js["GraphWindows"] = jsW;

    json["WindowGeometries"] = js;
}

void MainWindow::readWindowsFromJson(QJsonObject &json)
{
    QJsonObject js;
    parseJson(json, "WindowGeometries", js);
    if (!js.isEmpty())
    {
        QJsonObject jsMain = js["Main"].toObject();
        int x=10, y=10, w=500, h=700;
        bool bVis=true;
        LoadWindowFromJson(jsMain, x, y, w, h, bVis);
        setGeometry(x, y, w, h);

        QJsonObject jsScript;
        parseJson(js, "ScriptWindow", jsScript);
        if (!jsScript.isEmpty())
        {
            LoadWindowFromJson(jsScript, x, y, w, h, bVis);
            ScriptWindow->setGeometry(x, y, w, h);
            ScriptWindow->setVisible(bVis);
        }

#ifdef CERN_ROOT
        QJsonObject jsW;
        parseJson(js, "GraphWindows", jsW);
        RootModule->SetWindowGeometries(jsW);
#endif
    }
}



// --- Update GUI controls on Config change ---
void MainWindow::UpdateGui()
{
    qDebug() << "--- Updating GUI";

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

    ui->cobSignalExtractionMethod->setCurrentIndex(Config->SignalExtractionMethod);
    ui->sbExtractAllFromSampleNumber->setValue(Config->CommonSampleNumber);

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
