#ifndef TMPOBJHUBCLASS_H
#define TMPOBJHUBCLASS_H

#include <QString>
#include <QVector>
#include <QObject>

class TObject;

class RootDrawObj
{
  public:
    TObject* Obj;
    QString name; // it is also the title
    QString type; // object type (e.g. TH1D)

    QString Xtitle, Ytitle, Ztitle;
    int MarkerColor, MarkerStyle, MarkerSize, LineColor, LineStyle, LineWidth;

    RootDrawObj();
    ~RootDrawObj();
};

class ScriptDrawCollection
{
public:
   QVector<RootDrawObj> List;

   int findIndexOf(QString name); //returns -1 if not found
   bool remove(QString name);
   void append(TObject* obj, QString name, QString type);
   void clear() {List.clear();}
   void removeAllHists();
   void removeAllGraphs();
};

//=================================================================

class TmpObjHubClass : public QObject
{
    Q_OBJECT
public:
    TmpObjHubClass() {}
  ~TmpObjHubClass();

  ScriptDrawCollection ScriptDrawObjects;

public slots:
  void Clear();

};

#endif // TMPOBJHUBCLASS_H
