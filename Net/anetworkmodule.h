#ifndef ANETWORKMODULE_H
#define ANETWORKMODULE_H

#include <QObject>
#include <QTimer>
#include <QHostAddress>

class AWebSocketSessionServer;
class TObject;
class AScriptManager;

class ANetworkModule : public QObject
{
    Q_OBJECT
public:
    ANetworkModule();
    ~ANetworkModule();

    void SetDebug(bool flag) {fDebug = flag;}
    void SetScriptManager(AScriptManager* man);

    bool isWebSocketServerRunning() const;
    int getWebSocketPort() const;
    const QString getWebSocketServerURL() const;

    AWebSocketSessionServer* WebSocketServer = 0;

    void StartWebSocketServer(QHostAddress ip, quint16 port);
    void StopWebSocketServer();

public slots:
  void OnWebSocketTextMessageReceived(QString message);
  void OnClientDisconnected();

signals:
  void StatusChanged();
  void ReportTextToGUI(const QString text);

private:
  AScriptManager* ScriptManager = 0;
  bool fDebug = true;

};

#endif // ANETWORKMODULE_H
