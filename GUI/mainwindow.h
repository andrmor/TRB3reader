#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//#include <QMainWindow>
#include "aguiwindow.h"

#include <string>
#include <vector>

class MasterConfig;
class Trb3dataReader;
class Trb3signalExtractor;
class QTextStream;
class CernRootModule;
class ADispatcher;
class AHldFileProcessor;
class ANetworkModule;
class AServerMonitorWindow;
class ATrbRunControl;
class QTimer;
class QElapsedTimer;
class ABufferDelegate;
class QSpinBox;
class AGuiFromScrWin;
class AScriptWindow;

#ifdef TextToSpeechEnabled
class ATextToSpeech;
class ATextToSpeechConfigurator;
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public AGuiWindow
{
    Q_OBJECT

public:
    explicit MainWindow(ADispatcher* Dispatcher,
                        Trb3dataReader* Reader,
                        Trb3signalExtractor* Extractor,
                        AHldFileProcessor& HldFileProcessor,
                        ANetworkModule& Network,
                        QWidget *parent = nullptr);
    ~MainWindow();

    void SetEnabled(bool flag);

public slots:
    void UpdateGui();                               // slot since used by Dispatcher

    void ReadGUIfromJson(const QJsonObject &json);  // slot since used by Dispatcher
    void WriteGUItoJson(QJsonObject& json);         // slot since used by Dispatcher
    void SaveWindowSettings();                      // slot since used by Dispatcher   // !!!***
    void LoadWindowSettings();                      // slot since used by Dispatcher   // !!!***

    void onGlobalScriptStarted();
    void onGlobalScriptFinished();
    void saveCompleteState();     // !!!***

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
    void on_sbIntegrateFrom_editingFinished();
    void on_sbIntegrateTo_editingFinished();

    //menu actions
    void on_actionReset_positions_of_all_windows_triggered(); // !!!***
    void on_actionOpen_script_window_triggered();

    //Show/Hide Waveforms - triggered also by "close window" on the window itself!
    void on_pbShowWaveform_toggled(bool checked);
    void on_pbShowOverlayNeg_toggled(bool checked);
    void on_pbShowOverlayPos_toggled(bool checked);
    void on_pbShowAllNeg_toggled(bool checked);
    void on_pbShowAllPos_toggled(bool checked);
    void on_pbShowSignalsNegative_toggled(bool checked);
    void on_pbShowSignalsPositive_toggled(bool checked);
    void on_pbShowAllNegatives_toggled(bool checked);
    void on_pbShowAllPositives_toggled(bool checked);

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
    void on_cobLableType_activated(int index);
    void on_sbNumChannels_editingFinished();
    void on_sbNumSamples_editingFinished();
    void on_leAddToProcessed_editingFinished();
    void on_cobPedestalExtractionMethod_activated(int index);
    void on_ledPedestalPeakSigma_editingFinished();
    void on_ledPedestalPeakThreshold_editingFinished();
    void on_actionConfigure_WebSocket_server_triggered();

    void on_pbBoardOn_clicked();
    void on_pbBoardOff_clicked();

protected:
    void closeEvent(QCloseEvent* event);

private:
    MasterConfig        & Config;
    ADispatcher         * Dispatcher;
    Trb3dataReader      * Reader;
    Trb3signalExtractor * Extractor;
    AHldFileProcessor   & HldFileProcessor;
    ANetworkModule      & Network;

    //owned objects
    Ui::MainWindow* ui;
    CernRootModule * RootModule = nullptr;
    AServerMonitorWindow * ServerWindow = nullptr;

    AGuiFromScrWin * GuiFromScrWin = nullptr;
    AScriptWindow * JScriptWin = nullptr;

    //gui misc
    bool bStopFlag;
    bool bNeverRemindAppendToHub = false;
    //int  numProcessedEvents;
    //int  numBadEvents;

    QTimer * watchdogTimer = nullptr;
    QTimer * aTimer = nullptr;
    QElapsedTimer * elTimer = nullptr;
    bool bLimitMaxEvents = false;
    int MaxEventsToRun = 0;
    QTimer * timerAutoFreeSpace = nullptr;
    bool bAlreadyStopping = false;

#ifdef TextToSpeechEnabled
    ATextToSpeech * TextToSpeechHub = nullptr;
    ATextToSpeechConfigurator * TextToSpeechWindow = nullptr;
#endif

private:
    const QString ProcessData(); //returns error message if any
    void LogMessage(const QString message);
    bool saveSignalsToFile(const QString FileName, bool bUseHardware);
    bool sendSignalData(QTextStream& outStream, bool bUseHardware = false);

    void onEventChanged(int arg1);
    void onChannelChanged();
    void OnEventOrChannelChanged();
    void showOverlay(bool checked, bool bNeg);
    void showSignals(bool checked, bool bNeg);
    int  getCurrentlySelectedHardwareChannel();
    void showAllWave(bool checked, bool bNeg);
    void updateSmoothAfterPedeEnableStatus();

    void ClearData();
    void CreateScriptWindow();

    const QString PackChannelList(QVector<int> vec);
    const QString PackMappingList(QVector<int> vec);
    bool ExtractNumbersFromQString(const QString input, QVector<int>* ToAdd);
    //bool bulkProcessCore();
    void bulkProcessorEnvelope(const QStringList FileNames); // !!!***
    void updateNumEventsIndication();

private:
    ATrbRunControl * TrbRunManager = nullptr;
    int ZeroRateCounter = 0;

    int    TellRate_NumAverage = 5;
    int    TellRate_NumCurrent = 0;
    double TellRate_SoFarAccumulated = 0;

    std::vector<QSpinBox*> TriggerGainSpinBoxes;

    QString intToBitString(int val);
    QString intToBitStringShift1(int val);
    void updateTriggerGainGui();

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
    void on_cbTrapezoidal_clicked(bool checked);
    void on_sbTrapezoidalL_editingFinished();
    void on_sbTrapezoidalG_editingFinished();
    void on_cbZeroSignalIfPeakOutside_P_clicked(bool checked);
    void on_sbZeroSignalIfPeakBefore_P_editingFinished();
    void on_sbZeroSignalIfPeakAfter_P_editingFinished();
    void on_cbZeroSignalIfPeakOutside_N_clicked(bool checked);
    void on_sbZeroSignalIfPeakBefore_N_editingFinished();
    void on_sbZeroSignalIfPeakAfter_N_editingFinished();
    void on_sbEvent_editingFinished();

    void on_leFPGA3_0_editingFinished();
    void on_leFPGA3_1_editingFinished();
    void on_leFPGA4_0_editingFinished();
    void on_leFPGA4_1_editingFinished();
    void on_leTimeChannelsFPGA3_editingFinished();
    void on_leTimeChannelsFPGA4_editingFinished();
    void on_ledTimeWinBefore_FPGA3_editingFinished();
    void on_ledTimeWinAfter_FPGA3_editingFinished();
    void on_ledTimeWinBefore_FPGA4_editingFinished();
    void on_ledTimeWinAfter_FPGA4_editingFinished();
    void on_pbWriteTimeSettingsToTrb_clicked();
    void on_pbReadTimeSettingsFromTrb_clicked();
    void on_cbTimeEnable_FPGA3_clicked(bool checked);
    void on_cbTimeEnable_FPGA4_clicked(bool checked);
    void on_pbLoadLastAndProcess_clicked();
    void on_sbNumberTriggerBoardChannels_editingFinished();
    void on_pbSendTriggerBoardGains_clicked();
    void on_pbSetAllTriggerGainsTo_clicked();
    void on_cbGainsForTriggerBoard_clicked(bool checked);
    void on_sbAllGainsTo_editingFinished();

    void storeTriggerGainSettings();
    void on_cbDisableIgnoredChannels_clicked(bool checked);
    void on_pbAddTimingDatakind_clicked();
    void on_pbRemoveTimingDatakind_clicked();
    void on_cobWhatToSave_activated(int index);
    void on_cbAddRunTime_clicked(bool checked);
    void on_pbGotoFirstEvent_clicked();
    void on_pbGotoPreviousEvent_clicked();
    void on_pbGotoLastEvent_clicked();
    void on_pbGotoPreviousChannel_clicked();
    void on_sbChannel_editingFinished();
    void on_actionConfigure_triggered();
    void on_cbTellMeRate_customContextMenuRequested(const QPoint &pos);
    void on_leFolderForHldFiles_customContextMenuRequested(const QPoint &pos);
    void on_cbDoNotSaveDisabledChannels_clicked(bool checked);
    void on_cbSaveTime_clicked(bool checked);
};

#endif // MAINWINDOW_H
