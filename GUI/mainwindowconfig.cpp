#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "masterconfig.h"
#include "ajsontools.h"
#include "channelmapper.h"
#include "trb3signalextractor.h"
#include "trb3datareader.h"

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
    QJsonObject js;
#ifdef CERN_ROOT
    js = RootModule->SaveGraphWindows();
    delete RootModule; RootModule = 0;
#endif
    saveConfig(ConfigDir+"/autosave.json", js);
    QMainWindow::closeEvent(event);
}

void MainWindow::on_actionLoad_config_triggered()
{
    QString FileName = QFileDialog::getOpenFileName(this, "Load configuration", "", "Json files (*.json)");
    if (FileName.isEmpty()) return;

    ClearData();
    loadConfig(FileName);
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

    saveConfig(FileName, jsW);
}

void MainWindow::saveConfig(QString FileName, QJsonObject js)
{
    QJsonObject json;
    Config->WriteToJson(json);
    writeGUItoJson(json);
    writeWindowsToJson(json, js);

    SaveJsonToFile(json, FileName);
}

void MainWindow::loadConfig(QString FileName)
{
    QJsonObject json;
    LoadJsonFromFile(json, FileName);
    Config->ReadFromJson(json);
    readGUIfromJson(json);
    UpdateGui();
    readWindowsFromJson(json);

    Map->Clear();
    Map->SetChannels_OrderedByLogical(Config->ChannelMap);

    ClearData();
}

void MainWindow::writeGUItoJson(QJsonObject &json)
{
    QJsonObject jsgui;

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
    ui->ptePolarity->clear();
    QString s;
    for (int i: Config->NegativeChannels)
        s += QString::number(i)+" ";
    ui->ptePolarity->appendPlainText(s);

    ui->pteMapping->clear();
    s.clear();
    for (int i: Config->ChannelMap)
        s += QString::number(i)+" ";
    ui->pteMapping->appendPlainText(s);

    ui->cbSubstractPedestal->setChecked(Config->bPedestalSubstraction);
        ui->sbPedestalFrom->setValue(Config->PedestalFrom);
        ui->sbPedestalTo->setValue(Config->PedestalTo);

    ui->cbSmoothWaveforms->setChecked(Config->bSmoothWaveforms);
    ui->cbSmoothBeforePedestal->setChecked(Config->bSmoothingBeforePedestals);
        ui->cbAdjacentAveraging->setChecked(Config->AdjacentAveraging_bOn);
            ui->sbAdjAvPoints->setValue(Config->AdjacentAveraging_NumPoints);
            ui->cbAdjAvWeighted->setChecked(Config->AdjacentAveraging_bWeighted);

    ui->cbZeroSignalIfReverseMax->setChecked(Config->bZeroSignalIfReverse);
        ui->ledReverseMaxLimit->setText(QString::number(Config->ReverseMaxThreshold));
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

