#include "ainterfacetowebsocket.h"

#include <QWebSocketServer>
#include <QWebSocket>

#include <QApplication>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QElapsedTimer>

AInterfaceToWebSocket::AInterfaceToWebSocket()
{
    timeout = 3000;
    ClientSocket = new QWebSocket();
    connect(ClientSocket, &QWebSocket::connected, this, &AInterfaceToWebSocket::onClientConnected);
    connect(ClientSocket, &QWebSocket::textMessageReceived, this, &AInterfaceToWebSocket::onTextMessageReceived);
    State = Idle;

    Server = new QWebSocketServer(QStringLiteral("Echo Server"), QWebSocketServer::NonSecureMode, this);

    //qDebug() << QMetaObject::normalizedSignature(SIGNAL(RequestEvaluate(const QString&, QVariant&)));
}

AInterfaceToWebSocket::~AInterfaceToWebSocket()
{
    ClientSocket->deleteLater();

    Server->close();
    qDeleteAll(ServerListOfClients.begin(), ServerListOfClients.end());
}

QString AInterfaceToWebSocket::SendTextMessage(QString Url, QVariant message, bool WaitForAnswer)
{
   if (State != Idle)
   {
       qDebug() << "Not ready for this yet....";
       exit(678);
   }

   qDebug() << "Client: Attempting to send to Url:\n" << Url
            << "Message:\n->" << message << "<-";

   State = Sending;
   MessageToSend = variantToString(message);
   MessageReceived = "";
   fWaitForAnswer = WaitForAnswer;

   QElapsedTimer timer;
   timer.start();

   ClientSocket->open(QUrl(Url));   
   do
   {
       qApp->processEvents();
       if (timer.elapsed() > timeout)
       {
           ClientSocket->abort();
           State = Idle;
           abort("Timeout!");
           return "";
       }
   }
   while (State != Idle);

   ClientSocket->close();
   lastExchangeDuration = timer.elapsed();
   return MessageReceived;
}

QString AInterfaceToWebSocket::Ping(QString Url)
{
  SendTextMessage(Url, "", true);
  return QString("Ping time: ") + QString::number(lastExchangeDuration) + " ms";
}

void AInterfaceToWebSocket::onClientConnected()
{
    qDebug() << "Client: ClientSocket connected";
    ClientSocket->sendTextMessage(MessageToSend);

    if (!fWaitForAnswer) State = Idle;
    else State = WaitingForAnswer;
}

void AInterfaceToWebSocket::onTextMessageReceived(QString message)
{
    qDebug() << "Client: Message received:" << message;
    MessageReceived = message;
    State = Idle;
}

//--------------------- SERVER ------------------

bool AInterfaceToWebSocket::StartListen(quint16 port)
{
    if (Server->listen(QHostAddress::Any, port))
    {
            qDebug() << "Web socket server is now listening";
            qDebug() << "--Address:"<< Server->serverAddress();
            qDebug() << "--Port:"<< Server->serverPort();
            qDebug() << "--URL:" << Server->serverUrl().toString();

            connect(Server, &QWebSocketServer::newConnection, this, &AInterfaceToWebSocket::onNewConnection);

            return true;
    }
    else    return false;
}

void AInterfaceToWebSocket::StopListen()
{
    qDebug() << "Web socket server is not listening anymore!";
    Server->close();
}

void AInterfaceToWebSocket::onNewConnection()
{
    qDebug() << "Server: New connection established";
    QWebSocket *pSocket = Server->nextPendingConnection();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &AInterfaceToWebSocket::processTextMessage);
    //connect(pSocket, &QWebSocket::binaryMessageReceived, this, &AInterfaceToWebSocket::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &AInterfaceToWebSocket::socketDisconnected);

    ServerListOfClients << pSocket;
}

void AInterfaceToWebSocket::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Server: Message received:" << message << " from client:"<<pClient;
    ServerReplyClient = pClient;

    if (message.isEmpty()) //we were pinged!
      {
        ReplyAndClose("ping!");
        return;
      }

    QVariant res;
    emit RequestEvaluate(message, res);

    qDebug() << "Server: Evaluation finished, result:" << res;
    QString strRes = variantToString(res);
    qDebug() << "Server: Converted to string:"<<strRes;

    ReplyAndClose(strRes);
}

void AInterfaceToWebSocket::ReplyAndClose(QString message)
{
    qDebug() << "Server: Reply and close request";
    qDebug() << "Server: Message:"<<message<<"client:"<<ServerReplyClient;

    if (ServerReplyClient)
    {
        ServerReplyClient->sendTextMessage(message);
        ServerReplyClient->close();
        ServerReplyClient = 0;
    }
}


/*
void AInterfaceToWebSocket::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Binary Message received:" << message;
    //if (pClient) pClient->sendBinaryMessage(message);
}
*/

void AInterfaceToWebSocket::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    //  qDebug() << "Server: socketDisconnected:" << pClient;

    if (pClient)
    {
        ServerListOfClients.removeAll(pClient);
        pClient->deleteLater();
    }

    ServerReplyClient = 0;
}

//-------------- utilities ------------
const QString AInterfaceToWebSocket::variantToString(QVariant val) const
{
    QString type = val.typeName();

    QJsonValue jv;
    QString rep;
    if (type == "int")
      {
        jv = QJsonValue(val.toInt());
        rep = QString::number(val.toInt());
      }
    else if (type == "double")
      {
        jv = QJsonValue(val.toDouble());
        rep = QString::number(val.toDouble());
      }
    else if (type == "bool")
      {
        jv = QJsonValue(val.toBool());
        rep = val.toBool() ? "true" : "false";
      }
    else if (type == "QString")
      {
        jv = QJsonValue(val.toString());
        rep = val.toString();
      }
//    else if (type == "QVariantList")
//      {
//        QVariantList vl = val.toList();
//        QJsonArray ar = QJsonArray::fromVariantList(vl);
//        jv = ar;
//        rep = "-Array-";
//      }
    else if (type == "QVariantMap")
        {
          QVariantMap mp = val.toMap();
          QJsonObject json = QJsonObject::fromVariantMap(mp);
          QJsonDocument doc(json);
          rep = QString(doc.toJson(QJsonDocument::Compact));
        }

    return rep;
}

