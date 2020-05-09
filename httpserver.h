#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QVector>
#include <QMap>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>

struct HttpRequest
{
    QString method;
    QString uri;
    QString http_version;
    QMap<QString, QString> headers;
    QString body;
};

struct HttpResponse
{
    QString http_version;
    int status_code;
    QString status_description;
    QMap<QString, QString> headers;
    QString body;
};

class HttpServer : public QObject
{
   QTcpServer tcp_server;

   using HttpHandler = std::function<void (HttpRequest & req, HttpResponse & res)>;

   HttpHandler handler_ = nullptr;
   QMap<QString, HttpHandler> handlers_;

   HttpResponse handleRequest(HttpRequest & req);

   Q_OBJECT
public:
   explicit HttpServer(QObject * parent = nullptr);

   bool start(int port);

   void setHandler(const HttpHandler & handler)
   {
      handler_ = handler;
   }

   void get(const QString & uri, const HttpHandler & handler)
   {
       handlers_["GET " + uri] = handler;
   }

   void post(const QString & uri, const HttpHandler & handler)
   {
       handlers_["POST " + uri] = handler;
   }

   void put(const QString & uri, const HttpHandler & handler)
   {
       handlers_["PUT " + uri] = handler;
   }

   void delete_(const QString & uri, const HttpHandler & handler)
   {
       handlers_["DELETE " + uri] = handler;
   }

public slots:
   void onError(QAbstractSocket::SocketError socketError);
   void onNewConnection();
   void onClientReadyRead();
   void onClientDataSent();
   void onClientDisconnected();
};

#endif // HTTPSERVER_H
