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
    //
    QMap<QString, QString> params;
    QMap<QString, QString> query;
    //
    QString contentType() const;
    int contentLength() const;
};

struct HttpResponse
{
    QString http_version;
    int status_code;
    QString status_description;
    QMap<QString, QString> headers;
    QString body;
    //
    void setContentType(const QString & type);
    void setContentLength(int length);
};

using HttpHandler = std::function<void (HttpRequest & req, HttpResponse & res)>;

class HttpServer : public QObject
{
   QTcpServer tcp_server;
   HttpRequest current_request;

   HttpHandler handler_ = nullptr;
   QMap<QString, HttpHandler> handlers_;

   HttpHandler getHandler(HttpRequest & req);
   HttpResponse handleRequest(HttpRequest & req);

   Q_OBJECT
public:
   explicit HttpServer(QObject * parent = nullptr);

   bool start(int port);

   void setHandler(const HttpHandler & handler)
   {
      handler_ = handler;
   }

   void use(const QString & method, const QString & templ, const HttpHandler & handler)
   {
       handlers_[method + " " + templ] = handler;
   }

   void get(const QString & uri, const HttpHandler & handler) { use("GET", uri, handler); }

   void post(const QString & uri, const HttpHandler & handler) { use("POST", uri, handler); }

   void put(const QString & uri, const HttpHandler & handler) { use("PUT", uri, handler); }

   void delete_(const QString & uri, const HttpHandler & handler) { use("DELETE", uri, handler); }

private slots:
   void onError(QAbstractSocket::SocketError socketError);
   void onNewConnection();
   void onClientReadyRead();
   void onClientDataSent();
   void onClientDisconnected();
};

#endif // HTTPSERVER_H
