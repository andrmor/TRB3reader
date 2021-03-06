#include "anetworkmodule.h"
#include "ascriptmanager.h"
#include "awebsocketsessionserver.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>

ANetworkModule::ANetworkModule(AScriptManager *ScriptManager) : ScriptManager(ScriptManager)
{
    WebSocketServer = new AWebSocketSessionServer();

    QObject::connect(WebSocketServer, &AWebSocketSessionServer::textMessageReceived, this, &ANetworkModule::OnWebSocketTextMessageReceived);
    QObject::connect(WebSocketServer, &AWebSocketSessionServer::reportToGUI, this, &ANetworkModule::ReportTextToGUI);
    QObject::connect(WebSocketServer, &AWebSocketSessionServer::clientDisconnected, this, &ANetworkModule::OnClientDisconnected);

}

ANetworkModule::~ANetworkModule()
{
    delete HttpManager;
    delete WebSocketServer;
}

bool ANetworkModule::isWebSocketServerRunning() const
{
    return WebSocketServer->IsRunning();
}

int ANetworkModule::getWebSocketPort() const
{
    if (!WebSocketServer) return 0;
    return WebSocketServer->GetPort();
}

const QString ANetworkModule::getWebSocketServerURL() const
{
  if (!WebSocketServer) return "";
  return WebSocketServer->GetUrl();
}

void ANetworkModule::StartWebSocketServer(QHostAddress ip, quint16 port)
{    
    WebSocketServer->StartListen(ip, port);
    emit StatusChanged();
}

void ANetworkModule::StopWebSocketServer()
{
    WebSocketServer->StopListen();
    qDebug() << "Web socket server has stopped listening";
    emit StatusChanged();
}

void ANetworkModule::OnWebSocketTextMessageReceived(QString message)
{
    qDebug() << "Websocket server: Message (script) received";
    qDebug() << "  Evaluating as JavaScript";

    int line = ScriptManager->FindSyntaxError(message);
    if (line != -1)
    {
        qDebug() << "Syntaxt error!";
        WebSocketServer->sendError("  Syntax check failed - message is not a valid JavaScript");
    }
    else
    {
        QString res = ScriptManager->Evaluate(message);
        qDebug() << "  Script evaluation result:"<<res;

        if (ScriptManager->isEvalAborted())
        {
            qDebug() << "  Was aborted:" << ScriptManager->getLastError();
            WebSocketServer->sendError("Aborted -> " + ScriptManager->getLastError());
        }
        else
        {
            if ( !WebSocketServer->isReplied() )
            {
                if (res == "undefined") WebSocketServer->sendOK();
                else WebSocketServer->ReplyWithText("{ \"result\" : true, \"evaluation\" : \"" + res + "\" }");
            }
        }
    }
}

void ANetworkModule::OnClientDisconnected()
{
    qDebug() << "Client disconnected";
}

void ANetworkModule::replyFinished(QNetworkReply * reply)
{
    if (reply->error())
    {
        HttpError = reply->errorString();
        bHttpError = true;
    }
    else HttpReply = reply->readAll();

    reply->deleteLater();
    bHttpReplyReceived = true;
}

#include <QElapsedTimer>
#include <QApplication>
#include <QThread>
bool ANetworkModule::makeHttpRequest(const QString & url, QString & replyOrError, int timeout_ms)
{
    if (HttpManager) delete HttpManager;
    HttpManager = new QNetworkAccessManager();
    connect(HttpManager, &QNetworkAccessManager::finished, this, &ANetworkModule::replyFinished);

    bHttpReplyReceived = false;
    bHttpError = false;
    HttpReply.clear();
    HttpError.clear();

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    HttpManager->get(request);

    QElapsedTimer * elap = new QElapsedTimer();
    elap->start();
    while (!bHttpReplyReceived)
    {
        qApp->processEvents();
        if (elap->elapsed() > timeout_ms) break;
        QThread::usleep(100);
    }

    delete HttpManager; HttpManager = nullptr;
    if (!bHttpReplyReceived)
    {
        replyOrError = "Timeout";
        return false;
    }
    if (!HttpError.isEmpty())
    {
        replyOrError = HttpError;
        return false;
    }
    replyOrError = HttpReply;
    return true;
}
