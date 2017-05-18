#include "httphandler.h"

HTTPHandler::HTTPHandler(){

}

HTTPHandler::~HTTPHandler(){
    reply->deleteLater();
}

QByteArray HTTPHandler::wget(QUrl url, QString header, QList<QByteArray> &cookies){
    QNetworkRequest request(url);
    QStringList headers = header.split('\n');
    for(int i = 0; i < headers.length(); i++){
        if(headers.at(i).contains(':')){
            int pos = QString(headers.at(i)).indexOf(':');
            if(pos != -1){
                QString index = headers.at(i).mid(0, pos);
                QString value = headers.at(i).mid(pos+1).trimmed();
                request.setRawHeader( index.toLatin1() , value.toLatin1() );
            }
        }
    }
    if(cookies.length() > 0)
        request.setRawHeader("Cookie", cookies.join("; "));
    QEventLoop *loop = new QEventLoop;
    reply = qnam.get(request);
    connect(reply, &QNetworkReply::finished, (loop), &QEventLoop::quit);
    loop->exec();

    if (reply->error() == QNetworkReply::NoError) {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode == 301 || statusCode == 302 || statusCode == 307 || statusCode == 308) {
            QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
            delete loop;
            if(redirectUrl.isValid())
                return wget(redirectUrl.toString(), header, cookies);
            else
                return QByteArray();
        }else{
            QList<QByteArray> headerList = reply->rawHeaderList();
            QList<QByteArray> tmp;
            foreach(QByteArray head, headerList) {
                if(QString(head).contains("set-cookie", Qt::CaseInsensitive)){
                    QList<QByteArray> tmp2 = reply->rawHeader(head).split('\n');
                    foreach(QByteArray cookie, tmp2) {
                        QList<QByteArray> tmp3 = cookie.split(';');
                        foreach(QByteArray cookie_pair, tmp3) {
                            if(cookie_pair.indexOf('=') != -1 &&
                                    !QString(cookie_pair.split('=')[0]).contains("path", Qt::CaseInsensitive) &&
                                    !QString(cookie_pair.split('=')[0]).contains("domain", Qt::CaseInsensitive)){
                                tmp.append(cookie_pair);
                                break;
                            }
                        }
                    }
                }
            }
            if(tmp.length() >= cookies.length() )
                cookies = tmp;

            QByteArray testbyteary = reply->readAll();
            delete loop;
            if(testbyteary.mid(0,2) == QByteArray("\x1f\x8b",2)){
                return gUncompress(testbyteary);
            }else{
                return testbyteary;
            }
        }
    }else{
        delete loop;
        return QByteArray();
    }
}

QByteArray HTTPHandler::gUncompress(const QByteArray &data){
    if (data.size() <= 4) {
        qDebug() << "gUncompress: Input data is truncated";
        return QByteArray();
    }

    QByteArray result;

    int ret;
    z_stream strm;
    static const int CHUNK_SIZE = 1024;
    char out[CHUNK_SIZE];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = data.size();
    strm.next_in = (Bytef*)(data.data());

    ret = inflateInit2(&strm, 15 +  32); // gzip decoding
    if (ret != Z_OK)
        return QByteArray();

    // run inflate()
    do {
        strm.avail_out = CHUNK_SIZE;
        strm.next_out = (Bytef*)(out);

        ret = inflate(&strm, Z_NO_FLUSH);
        Q_ASSERT(ret != Z_STREAM_ERROR);  // state not clobbered

        switch (ret) {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR;     // and fall through
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)inflateEnd(&strm);
            return QByteArray();
        }

        result.append(out, CHUNK_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);

    // clean up and return
    inflateEnd(&strm);
    return result;
}
