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
class ADataHub;
class AHldFileProcessor;
class ANetworkModule;
class AServerMonitorWindow;
class ATrbRunControl;
class QTimer;
class QElapsedTimer;
class ABufferDelegate;
class AInterfaceToSpeech;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(MasterConfig* Config,
                        ADispatcher* Dispatcher,
                        ADataHub* DataHub,
                        Trb3dataReader* Reader,
                        Trb3signalExtractor* Extractor,
                        AHldFileProcessor& HldFileProcessor,
                        ANetworkModule& Network,
                        QWidget *parent = 0);
    ~MainWindow();

    void SetEnabled(bool flag);

public slots:
    void UpdateGui();                               // slot since used by Dispatcher

    void ReadGUIfromJson(const QJsonObject &json);  // slot since used by Dispatcher
    void WriteGUItoJson(QJsonObject& json);         // slot since used by Dispatcher
    void SaveWindowSettings();                      // slot since used by Dispatcher
    void LoadWindowSettings();                      // slot since used by Dispatcher

    void onGlobalScriptStarted();
    void onGlobalScriptFinished();
    void saveCompleteState();

    void onShowMessageRequest(const QString message);
    void onShowActionRequest(const QString action);
    void onProgressUpdate(int progress);
private slots:
    //right-click menus
    void on_ptePolarity_customContextMenuRequested(const QPoint &pos);
    void on_pteMapping_customContextMenuRequested(const QPoint &pos);
    void on_pteIgnoreHardwareChannels_customContextMenuRequested(const QPoint &pos);

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
    void on_pbSelectFile_clicked();
    void on_pbProcessData_clicked();
    void on_pbSaveTotextFile_clicked();
    void on_pbStop_toggled(bool checked);
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
    void on_cbPosMaxSignalGate_clicked(bool checked);
    void on_cbNegMaxSignalGate_clicked(bool checked);
    void on_sbPosMaxFrom_editingFinished();
    void on_sbPosMaxTo_editingFinished();
    void on_sbNegMaxFrom_editingFinished();
    void on_sbNegMaxTo_editingFinished();
    void on_sbExtractAllFromSampleNumber_editingFinished();
    void on_pbEditListOfNegatives_clicked();
    void on_pbEditMap_clicked();
    void on_pbEditIgnoreChannelList_clicked();
    void on_pbAddDatakind_clicked();
    void on_pbRemoveDatakind_clicked();
    void on_pbPrintHLDfileProperties_clicked();
    void on_pbProcessAllFromDir_clicked();
    void on_pbProcessSelectedFiles_clicked();
    void on_pbSaveSignalsFromDataHub_clicked();
    void on_sbIntegrateFrom_editingFinished();
    void on_sbIntegrateTo_editingFinished();

    //menu actions
    void on_actionReset_positions_of_all_windows_triggered();
    void on_actionOpen_script_window_triggered();

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
    void on_cobSignalExtractionMethod_currentIndexChanged(int index);
    void on_cobExplorerSource_currentIndexChanged(int index);
    void on_pbClearDataHub_clicked();
    void on_pbLoadToDataHub_clicked();
    void on_cobLableType_activated(int index);
    void on_sbNumChannels_editingFinished();
    void on_sbNumSamples_editingFinished();
    void on_cbBulkExtract_clicked();
    void on_cbAutoExecuteScript_clicked();
    void on_cbSaveSignalsToFiles_clicked();
    void on_leAddToProcessed_editingFinished();
    void on_cbBulkCopyToDatahub_clicked();
    void on_cbBulkAlsoCopyWaveforms_clicked();
    void on_cobPedestalExtractionMethod_activated(int index);
    void on_ledPedestalPeakSigma_editingFinished();
    void on_ledPedestalPeakThreshold_editingFinished();
    void on_actionConfigure_WebSocket_server_triggered();

    void on_pbBoardOn_clicked();
    void on_pbBoardOff_clicked();

protected:
    void closeEvent(QCloseEvent* event);

private:
    //aliases    
    MasterConfig* Config;
    ADispatcher* Dispatcher;
    ADataHub* DataHub;
    Trb3dataReader* Reader;
    Trb3signalExtractor* Extractor;
    AHldFileProcessor& HldFileProcessor;
    ANetworkModule& Network;

    //owned objects
    Ui::MainWindow* ui;    
    AScriptWindow* ScriptWindow;
#ifdef CERN_ROOT
    CernRootModule* RootModule;
#endif
    AServerMonitorWindow* ServerWindow = 0;

    //gui misc
    bool bStopFlag;
    bool bNeverRemindAppendToHub = false;
    //int  numProcessedEvents;
    //int  numBadEvents;

    QTimer * watchdogTimer = 0;
    QTimer * aTimer = 0;
    QElapsedTimer * elTimer = 0;
    bool bLimitMaxEvents = false;
    int MaxEventsToRun = 0;
    QTimer * timerAutoFreeSpace = 0;
    bool bAlreadyStopping = false;

#ifdef SPEECH
    AInterfaceToSpeech* speech = 0;
#endif

private:
    const QString ProcessData(); //returns error message if any
    void LogMessage(const QString message);
    bool saveSignalsToFile(const QString FileName, bool bUseHardware);
    bool sendSignalData(QTextStream& outStream, bool bUseHardware = false);

    void OnEventOrChannelChanged();
    void showOverlay(bool checked, bool bNeg);
    int  getCurrentlySelectedHardwareChannel();
    void showAllWave(bool checked, bool bNeg);
    void updateSmoothAfterPedeEnableStatus();

    void ClearData();
    void CreateScriptWindow();

    const QString PackChannelList(QVector<int> vec);
    const QString PackMappingList(QVector<int> vec);
    bool ExtractNumbersFromQString(const QString input, QVector<int>* ToAdd);
    //bool bulkProcessCore();
    void bulkProcessorEnvelope(const QStringList FileNames);
    void updateNumEventsIndication();

private:
    ATrbRunControl * TrbRunManager = 0;
    int ZeroRateCounter = 0;

private slots:
    void onBoardLogNewText(const QString text);
    void onRequestClearLog();
    void on_pbStartAcquire_clicked();
    void on_pbStopAcquire_clicked();
    void onBoardIsAlive(double currentAccepetedRate);
    void onBoardDisconnected();
    void onAcquireIsAlive();
    void onAcquireOff();
    void onWatchdogFailed();
    void on_cbLimitedTime_clicked(bool checked);
    void on_cbLimitEvents_clicked(bool checked);
    void on_pbOpenCTS_clicked();
    void on_pbOpenBufferControl_clicked();

    void on_leUser_editingFinished();
    void on_leHost_editingFinished();
    void on_leStartupScriptOnHost_editingFinished();
    void on_leStorageXmlOnHost_editingFinished();
    void on_leFolderForHldFiles_editingFinished();
    void on_leiHldFileSize_editingFinished();
    void on_ledTimeSpan_editingFinished();
    void on_cobTimeUnits_activated(int index);
    void on_leiMaxEvents_editingFinished();
    void on_pbReadTriggerSettingsFromTrb_clicked();
    void on_pbUpdateStartup_clicked();
    void on_pbOpenCtsWebPage_clicked();
    void on_pbSendCTStoTRB_clicked();
    void on_leDirOnHost_editingFinished();
    void on_pbRefreshBufferIndication_clicked();
    void on_cbBufferReadFromTRB_clicked();
    void on_pbBufferSendToTRB_clicked();
    void on_pbBufferUpdateScript_clicked();

    void onBufferDeleagateChanged(ABufferDelegate *del);
    void onFreeSpaceReportReady(long bytes);
    void on_pbRestartTrb_clicked();
    void on_pbUpdateTriggerGui_clicked();
    void on_pbUpdateTriggerSettings_clicked();
    void on_pbOpenBufferWebPage_clicked();
    void on_cbAutocheckFreeSpace_toggled(bool checked);
    void onTimeLimitForAcquireReached();
};

#endif // MAINWINDOW_H
