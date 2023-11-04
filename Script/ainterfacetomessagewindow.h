#ifndef AINTERFACETOMESSAGEWINDOW_H
#define AINTERFACETOMESSAGEWINDOW_H

#include "ascriptinterface.h"

class QDialog;
class QPlainTextEdit;
class QWidget;

class AInterfaceToMessageWindow : public AScriptInterface
{
  Q_OBJECT

public:
  AInterfaceToMessageWindow(QWidget *parent);
  ~AInterfaceToMessageWindow();

public slots:
  void  setEnable(bool flag) {bEnabled = flag;}

  void  move(double x, double y);
  void  resize(double w, double h);
  void  show();
  void  hide();
  void  setTransparent(bool flag);

  void  append(const QString txt);
  void  show(const QString txt, int ms = -1);
  void  clear();

  void  setFontSize(int size);

public:
  void  deleteDialog();
  bool  isActive() {return bActivated;}
  void  hideDialog();     //does not affect bActivated status
  void  restore();  //does not affect bActivated status

private:
  QWidget*  Parent;
  bool      bActivated;

  QDialog   *D;
  double    X, Y;
  double    WW, HH;

  QPlainTextEdit* e;
  bool      bEnabled;

  void init(bool fTransparent);
};

#endif // AINTERFACETOMESSAGEWINDOW_H
