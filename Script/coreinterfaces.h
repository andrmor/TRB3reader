#ifndef COREINTERFACES_H
#define COREINTERFACES_H

#include "ascriptinterface.h"

#include <QVariant>
#include <QString>
#include <QDesktopServices>

class AScriptManager;
//class TRandom2;

class AInterfaceToCore : public AScriptInterface
{
  Q_OBJECT

public:
  AInterfaceToCore(AScriptManager *ScriptManager);

public slots:
  //abort execution of the script
  void          abort(const QString message = "Aborted!") const;

  QVariant      evaluate(const QString script);

  void          sleep(int ms);

  //output (lower) field of the script window
  void          print(const QString text);
  void          clearText();
  const QString str(double value, int precision);

  //time stamps
  const QString GetTimeStamp() const;
  const QString GetDateTimeStamp() const;

  //save to file
  bool          createFile(const QString fileName, bool AbortIfExists = true) const;
  bool          isFileExists(const QString fileName) const;
  bool          deleteFile(const QString fileName) const;
  bool          createDir(const QString path) const;
  const QString getCurrentDir() const;
  bool          setCirrentDir(const QString path) const;
  bool          save(const QString fileName, QString str) const;
  bool          saveArray(const QString fileName, const QVariant array) const;
  bool          saveObject(const QString FileName, const QVariant Object, bool CanOverride) const;

  //load from file
  const QVariant loadColumn(const QString fileName, int column = 0) const; //load column of doubles from file and return it as an array
  const QVariant loadArray(const QString fileName, int columns) const;     //load column of doubles from file and return it as an array
  const QString  loadText(const QString fileName) const;

  //dirs
  const QString  GetWorkDir() const;
  const QString  GetScriptDir() const;
  const QString  GetExamplesDir() const;

  //externals
  const QString  StartExternalProcess(QString command, QVariant arguments, bool waitToFinish = false, int milliseconds = 1000);

private:
  AScriptManager* ScriptManager;
};

// ---- M A T H ----
class AInterfaceToMath : public AScriptInterface
{
  Q_OBJECT
  Q_PROPERTY(double pi READ pi)
  double pi() const { return 3.141592653589793238462643383279502884; }

public:
  AInterfaceToMath();
  //AInterfaceToMath(TRandom2* RandGen);
  //void setRandomGen(TRandom2* RandGen);

public slots:
  double abs(double val);
  double acos(double val);
  double asin(double val);
  double atan(double val);
  double atan2(double y, double x);
  double ceil(double val);
  double cos(double val);
  double exp(double val);
  double floor(double val);
  double log(double val);
  double max(double val1, double val2);
  double min(double val1, double val2);
  double pow(double val, double power);
  double sin(double val);
  double sqrt(double val);
  double tan(double val);
  double round(double val);
  /*
  double random();
  double gauss(double mean, double sigma);
  double poisson(double mean);
  double maxwell(double a);  // a is sqrt(kT/m)
  double exponential(double tau);
  */

private:
  //TRandom2* RandGen;
};

#endif // COREINTERFACES_H
