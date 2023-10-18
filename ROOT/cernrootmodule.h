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

    TMultiGraph *multiGraph;
    TGraph* gSingle;
    QVector<TGraph*> graphsNeg, graphsPos;

    AGraphWindow * WOne     = nullptr;
    AGraphWindow * WOverNeg = nullptr;
    AGraphWindow * WOverPos = nullptr;
    AGraphWindow * WAllNeg  = nullptr;
    AGraphWindow * WAllPos  = nullptr;

    int NormalColor;
    int RejectedColor;
    int LineWidth;

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

};

#endif // CERNROOTMODULE_H
