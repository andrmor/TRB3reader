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
class TGraph;
class ChannelMapper;

class CernRootModule : public QObject
{
    Q_OBJECT

public:
    CernRootModule(Trb3dataReader* Reader, Trb3signalExtractor* Extractor, ChannelMapper* Map, int refreshInterval = 100);
    ~CernRootModule();

    //Graph windows show/hide
    void ShowSingleWaveWindow(bool flag);
    void ShowOverNegWaveWindow(bool flag);
    void ShowOverPosWaveWindow(bool flag);
    void ShowAllNegWaveWindow(bool flag);
    void ShowAllPosWaveWindow(bool flag);

    void ClearSingleWaveWindow();

    void DrawSingle(int ievent, int iHardwChan, bool autoscale, double MinY, double MaxY);
    void DrawOverlay(int ievent, bool bNeg, bool bAutoscale, double Min, double Max, int SortBy_0Logic1Hardw);
    void DrawAll(int ievent, bool bNeg, int padsX, int padsY, bool bAutoscale, double Min, double Max, int SortBy_0Logic1Hardw, bool bShowlabels);

    void StartGraphWindows();
    const QJsonObject SaveGraphWindows() const;
    void SetWindowGeometries(const QJsonObject &js);
    void ResetPositionOfWindows();

private:
    Trb3dataReader* Reader;
    Trb3signalExtractor* Extractor;
    ChannelMapper* Map;

    TApplication* RootApp;

    TMultiGraph *multiGraph;
    TGraph* gSingle;
    QVector<TGraph*> graphsNeg, graphsPos;

    AGraphWindow *WOne, *WOverNeg, *WOverPos, *WAllNeg, *WAllPos;

    int NormalColor;
    int RejectedColor;
    int LineWidth;

    void showGraphWindow(AGraphWindow **win, bool flag);
    void clearNegGraphVectors();
    void clearPosGraphVectors();

private slots:
    void timerTimeout();

signals:
    void WOneHidden();
    void WOverNegHidden();
    void WOverPosHidden();
    void WAllNegHidden();
    void WAllPosHidden();

};

#endif // CERNROOTMODULE_H
