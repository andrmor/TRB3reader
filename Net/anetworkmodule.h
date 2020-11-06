#ifndef ANETWORKMODULE_H
#define ANETWORKMODULE_H

#include <QObject>
#include <QTimer>
#include <QHostAddress>

class AWebSocketSessionServer;
class TObject;
class AScriptManager;
class QNetworkAccessManager;
class QNetworkReply;

class ANetworkModule : public QObject
{
    Q_OBJECT
public:
    ANetworkModule(AScriptManager* ScriptManager);
    ~ANetworkModule();

    AScriptManager* getScriptManager() {return ScriptManager;}
    void SetDebug(bool flag) {fDebug = flag;}

    bool isWebSocketServerRunning() const;
    int getWebSocketPort() const;
    const QString getWebSocketServerURL() const;

    AWebSocketSessionServer* WebSocketServer = 0;

    void StartWebSocketServer(QHostAddress ip, quint16 port);
    void StopWebSocketServer();

    bool makeHttpRequest(const QString & url, QString & replyOrError, int timeout_ms);

public slots:
  void OnWebSocketTextMessageReceived(QString message);
  void OnClientDisconnected();

private slots:
  void replyFinished(QNetworkReply *reply);

signals:
  void StatusChanged();
  void ReportTextToGUI(const QString text);

private:
  AScriptManager* ScriptManager;
  bool fDebug = true;

  QNetworkAccessManager * HttpManager = nullptr;
  bool bHttpReplyReceived = false;
  bool bHttpError = false;
  QString HttpReply;
  QString HttpError;

};

#endif // ANETWORKMODULE_H
