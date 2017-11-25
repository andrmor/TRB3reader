#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>

class MasterConfig;
class Trb3dataReader;
class Trb3signalExtractor;
class QTextStream;
class CernRootModule;
class AScriptWindow;
class ADispatcher;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void writeGUItoJson(QJsonObject& json);
    void readGUIfromJson(QJsonObject& json);
    void writeWindowsToJson(QJsonObject &json, const QJsonObject jsW);
    void readWindowsFromJson(QJsonObject &json);

    void UpdateGui();

public slots:
    void onGlobalScriptStarted();
    void onGlobalScriptFinished();
    void saveCompleteState();

private slots:
    //right-click menus
    void on_ptePolarity_customContextMenuRequested(const QPoint &pos);
    void on_pteMapping_customContextMenuRequested(const QPoint &pos);

    //Update Config on GUI operated by user
    void on_pbLoadPolarities_clicked();
    void on_pbAddMapping_clicked();
    void on_cbSubstractPedestal_clicked(bool checked);
    void on_sbPedestalFrom_editingFinished();
    void on_sbPedestalTo_editingFinished();
    void on_cbSmoothWaveforms_clicked(bool checked);
    void on_cbSmoothBeforePedestal_clicked(bool checked);
    void on_cbAdjacentAveraging_clicked(bool checked);
    void on_sbAdjAvPoints_editingFinished();
    void on_cbAdjAvWeighted_clicked(bool checked);
    void on_cbZeroSignalIfReverseMax_clicked(bool checked);
    void on_ledReverseMaxLimit_editingFinished();

    //main user actions
    void on_pbSelectFile_clicked();
    void on_pbProcessData_clicked();
    void on_pbSaveTotextFile_clicked();
    void on_pbBulkProcess_clicked();
    void on_pbStop_toggled(bool checked);

    //event / channel changed
    void on_sbEvent_valueChanged(int arg1);
    void on_sbChannel_valueChanged(int arg1);

    //Show Waveforms
    void on_pbShowWaveform_toggled(bool checked);
    void on_pbShowOverlayNeg_toggled(bool checked);
    void on_pbShowOverlayPos_toggled(bool checked);
    void on_pbShowAllNeg_toggled(bool checked);
    void on_pbShowAllPos_toggled(bool checked);

    //auto-redraws
    void on_pbGotoNextEvent_clicked();
    void on_pbGotoNextChannel_clicked();
    void on_sbAllNegX_editingFinished();
    void on_sbAllNegY_editingFinished();
    void on_sbAllPosX_editingFinished();
    void on_sbAllPosY_editingFinished();
    void on_ledMinNeg_editingFinished();
    void on_ledMaxNeg_editingFinished();
    void on_ledMinPos_editingFinished();
    void on_ledMaxPos_editingFinished();
    void on_cbLabels_clicked();
    void on_cobSortBy_activated(int index);
    void on_cbAutoscaleY_clicked();
    void on_cobHardwareOrLogical_activated(int index);


    //config save/load
    void on_actionLoad_config_triggered();
    void on_actionSave_config_triggered();


    //gui updates
    void on_cbSubstractPedestal_toggled(bool checked);
    void on_cbSmoothWaveforms_toggled(bool checked);




    void on_actionReset_positions_of_all_windows_triggered();

    void on_pbNegSignature_clicked();

    void on_cbPosThreshold_clicked(bool checked);

    void on_cbNegThreshold_clicked(bool checked);

    void on_ledPosThresholdMin_editingFinished();

    void on_ledPosIgnoreMax_editingFinished();

    void on_ledNegThresholdMin_editingFinished();

    void on_ledNegIgnoreMax_editingFinished();

    void on_cbIgnorePosThreshold_clicked(bool checked);

    void on_cbIgnoreNegThreshold_clicked(bool checked);

    void on_cobSignalExtractionMethod_activated(int index);

    void on_pbAddListHardwChToIgnore_clicked();

    void on_pteIgnoreHardwareChannels_customContextMenuRequested(const QPoint &pos);

    void on_cbPosMaxSignalGate_clicked(bool checked);

    void on_cbNegMaxSignalGate_clicked(bool checked);

    void on_sbPosMaxFrom_editingFinished();

    void on_sbPosMaxTo_editingFinished();

    void on_sbNegMaxFrom_editingFinished();

    void on_sbNegMaxTo_editingFinished();

    void on_pbPosSignature_clicked();

    void on_sbExtractAllFromSampleNumber_editingFinished();

    void on_cobSignalExtractionMethod_currentIndexChanged(int index);

    void on_actionOpen_script_window_triggered();

    void on_pbSelectNewDir_clicked();

    void on_pbEditListOfNegatives_clicked();

protected:
    void closeEvent(QCloseEvent* event);

private:
    Ui::MainWindow *ui;
    MasterConfig* Config;    
    AScriptWindow* ScriptWindow;

    Trb3dataReader* Reader;
    Trb3signalExtractor* Extractor;

    ADispatcher* Dispatcher;

#ifdef CERN_ROOT
    CernRootModule* RootModule;
#endif

    bool bStopFlag;


    const QString ProcessData(); //returns error message if any
    void LogMessage(QString message);
    bool saveSignalsToFile(QString FileName, bool bUseHardware);
    bool sendSignalData(QTextStream& outStream, bool bUseHardware = false);

    void OnEventOrChannelChanged(bool bOnlyChannel = false);
    void showOverlay(bool checked, bool bNeg);
    int getCurrentlySelectedHardwareChannel();
    void showAllWave(bool checked, bool bNeg);
    void updateSmoothAfterPedeEnableStatus();

    void ClearData();
    void CreateScriptWindow();

    QString PackChannelList(QVector<int> vec);
    bool ExtractNumbersFromQString(const QString input, QList<int> *ToAdd);
};

#endif // MAINWINDOW_H
