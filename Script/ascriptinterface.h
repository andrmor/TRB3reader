#ifndef ASCRIPTINTERFACE_H
#define ASCRIPTINTERFACE_H

#include <QString>
#include <QHash>
#include <QObject>

class AScriptInterface : public QObject
{
  Q_OBJECT

public:
  AScriptInterface() {}
  AScriptInterface(const AScriptInterface& other) : QObject(), H(other.H), Description(other.Description) {}

  virtual bool InitOnRun() {return true;}                     // automatically called before script evaluation
  virtual void ForceStop() {}                                 // called when abort was triggered by any other module

  virtual bool IsMultithreadCapable() const {return false;}   // should be accessible in multithread script mode?
  const QString getDescription() const
    {return Description + (IsMultithreadCapable()?"\nMultithread-capable":"");}  // description text for the unit in GUI

public slots:
  const QString help(QString method) const                    //automatically requested to obtain help strings
  {
    if (method.endsWith("()")) method.remove("()");
    if (method.endsWith("(")) method.remove("(");
    if (!H.contains(method)) return "";
    return H[method];
  }

signals:
  void AbortScriptEvaluation(const QString) const;      //abort request is automatically linked to abort slot of core unit

protected:
  QHash<QString, QString> H;
  QString Description;

  void abort(const QString message = "Aborted!") const {emit AbortScriptEvaluation(message);}
};

#endif // ASCRIPTINTERFACE_H
