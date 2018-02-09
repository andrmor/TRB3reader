#ifndef HISTGRAPHINTERFACES_H
#define HISTGRAPHINTERFACES_H

#include "ascriptinterface.h"

#include <QString>
#include <QVariant>

class TmpObjHubClass;
class TObject;

// ---- H I S T O G R A M S ---- (TH1D of ROOT)
class AInterfaceToHist : public AScriptInterface
{
  Q_OBJECT

public:
  AInterfaceToHist(TmpObjHubClass *TmpHub);
  ~AInterfaceToHist(){}

  virtual bool IsMultithreadCapable() const {return true;}

public slots:
  void NewHist(QString HistName, int bins, double start, double stop);
  void NewHist2D(QString HistName, int binsX, double startX, double stopX, int binsY, double startY, double stopY);

  void SetTitles(QString HistName, QString X_Title, QString Y_Title, QString Z_Title = "");
  void SetLineProperties(QString HistName, int LineColor, int LineStyle, int LineWidth);
  void SetFillProperties(QString HistName, int color, int style);

  void Fill(QString HistName, double val, double weight);
  void Fill2D(QString HistName, double x, double y, double weight);

  void FillArr(QString HistName, QVariant Array);
  void Fill2DArr(QString HistName, QVariant Array);

  void Divide(QString HistName, QString HistToDivideWith);

  void Draw(QString HistName, QString options);
  void DrawStack(QVariant HistNames, QString options, QString XTitle = "", QString YTitle = "");

  void Save(QString HistName, QString FileName); // ***!!! add storage / delete!!!

  void Smooth(QString HistName, int times);
  QVariant FitGauss(QString HistName, QString options="");
  QVariant FitGaussWithInit(QString HistName, QVariant InitialParValues, QString options="");

  bool Delete(QString HistName);
  void DeleteAllHist();

  bool IsHistExist(QString HistName);

  void SetOptStat(QString opt);

signals:
  void RequestDraw(TObject* obj, QString options, bool fFocus);

private:
  TmpObjHubClass *TmpHub;
};

// ---- G R A P H S ---- (TGraph of ROOT)
class AInterfaceToGraph : public AScriptInterface
{
  Q_OBJECT

public:
  AInterfaceToGraph(TmpObjHubClass *TmpHub);
  ~AInterfaceToGraph(){}

  virtual bool IsMultithreadCapable() const {return true;}

public slots:
  void NewGraph(QString GraphName);
  void SetMarkerProperties(QString GraphName, int MarkerColor, int MarkerStyle, int MarkerSize);
  void SetLineProperties(QString GraphName, int LineColor, int LineStyle, int LineWidth);
  void SetTitles(QString GraphName, QString X_Title, QString Y_Title);

  void AddPoint(QString GraphName, double x, double y);
  void AddPoints(QString GraphName, QVariant xArray, QVariant yArray);
  void AddPoints(QString GraphName, QVariant xyArray);

  void Draw(QString GraphName, QString options);

  void Save(QString GraphName, QString FileName);

  bool Delete(QString GraphName);
  void DeleteAllGraph();

  bool IsGraphExists(QString GraphName);

signals:
  void RequestDraw(TObject* obj, QString options, bool fFocus);

private:
  TmpObjHubClass *TmpHub;
};

#endif // HISTGRAPHINTERFACES_H
