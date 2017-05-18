#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>

#include <zlib.h>

//#include "magicmgc.h"

class HTTPHandler : public QObject
{
public:
    HTTPHandler();
    ~HTTPHandler();
    QByteArray wget(QUrl currentUrl, QString header, QList<QByteArray> &cookies);

private:
    QNetworkAccessManager qnam;
    QNetworkReply *reply;

    QByteArray gUncompress(const QByteArray &data);

};

#endif // HTTPHANDLER_H
