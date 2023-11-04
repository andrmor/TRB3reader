#ifndef CERNROOTMODULE_H
#define CERNROOTMODULE_H

#include <QObject>
#include <QVector>
#include <QJsonObject>

class TApplication;
class TMultiGraph;
class AGraphWindow;
class Trb3dataReader;
class Trb3signalExtractor;
class MasterConfig;
class ADataHub;
class TGraph;
class TmpObjHubClass;
class TObject;
class TH2D;
class QMainWindow;

class CernRootModule : public QObject
{
    Q_OBJECT

public:
    CernRootModule(Trb3dataReader* Reader, Trb3signalExtractor* Extractor, MasterConfig* Config, ADataHub* DataHub, int refreshInterval = 100);
    ~CernRootModule();

    //Graph windows show/hide
    void ShowSingleWaveWindow(bool flag);
    void ShowOverNegWaveWindow(bool flag);
    void ShowOverPosWaveWindow(bool flag);
    void ShowAllNegWaveWindow(bool flag);
    void ShowAllPosWaveWindow(bool flag);
    void ShowNegativeSignalWindow(bool flag);
    void ShowPositiveSignalWindow(bool flag);

    void Show2DNegWindow(bool flag);
    void Show2DPosWindow(bool flag);

    void ClearSingleWaveWindow();
    void ClearOverNegWaveWindow();
    void ClearOverPosWaveWindow();
    void ClearAllNegWaveWindow();
    void ClearAllPosWaveWindow();

    bool DrawSingle(bool bFromDataHub, int ievent, int iHardwChan, bool autoscale, float MinY, float MaxY);
    bool DrawOverlay(bool bFromDataHub, int ievent, bool bNeg, bool bAutoscale, float Min, float Max, int SortBy_0Logic1Hardw);
    bool DrawAll(bool bFromDataHub, int ievent, bool bNeg, int padsX, int padsY,
                 bool bAutoscale, float Min, float Max, int SortBy_0Logic1Hardw,
                 bool bShowlabels, int Channels0_Signals1);
    void DrawSignals(bool bFromDataHub, int ievent, bool bNeg);
    void Draw2D(bool bNegatives, bool bFromDataHub, bool bAutoscale, double Min, double Max);

    void CreateGraphWindows();
    //const QJsonObject SaveGraphWindows() const;
    //void SetWindowGeometries(const QJsonObject &js);
    void ResetPositionOfWindows();

    void DrawSignature(bool bNeg);

    TmpObjHubClass* GetTmpHub() {return TmpHub;}

    void setMainWindow(QMainWindow * main) {MainWin = main;}

public slots:
    void onDrawRequested(TObject* obj, QString opt, bool bDoUpdate);

private:
    Trb3dataReader* Reader;
    Trb3signalExtractor* Extractor;
    MasterConfig* Config;
    ADataHub* DataHub;

    QMainWindow * MainWin = nullptr;

    TApplication* RootApp;

    TmpObjHubClass* TmpHub;

    TMultiGraph * multiGraph = nullptr;
    TGraph * gSingle = nullptr;
    QVector<TGraph*> graphsNeg, graphsPos;
    TGraph * gNegSig = nullptr;
    TGraph * gPosSig = nullptr;
    TH2D * h2DNeg = nullptr;
    TH2D * h2DPos = nullptr;

    AGraphWindow * WOne     = nullptr;
    AGraphWindow * WOverNeg = nullptr;
    AGraphWindow * WOverPos = nullptr;
    AGraphWindow * WAllNeg  = nullptr;
    AGraphWindow * WAllPos  = nullptr;
    AGraphWindow * WSigPos  = nullptr;
    AGraphWindow * WSigNeg  = nullptr;

    AGraphWindow * W2DNeg  = nullptr;
    AGraphWindow * W2DPos  = nullptr;

    int NormalColor = 4;
    int RejectedColor = 2;
    int LineWidth = 2;

    void showGraphWindow(AGraphWindow * win, bool flag);
    void clearNegGraphVectors();
    void clearPosGraphVectors();

    void SetGraphAttributes(TGraph* g, bool bFromDataHub, int ievent, int ichannel);

private slots:
    void timerTimeout();
    void onGraphWindowRequestHide(QString idStr);

signals:
    void WOneHidden();
    void WOverNegHidden();
    void WOverPosHidden();
    void WAllNegHidden();
    void WAllPosHidden();
    void WSigNegHidden();
    void WSigPosHidden();
    void W2DNegHidden();
    void W2DPosHidden();

};

#endif // CERNROOTMODULE_H
