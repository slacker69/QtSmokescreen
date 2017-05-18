#ifndef STHREAD_H
#define STHREAD_H

#include <QObject>
#include <QPlainTextEdit>
#include <QThread>
#include <QMutexLocker>
#include <QDebug>
#include <QDateTime>
#include <QVector>
#include <QMessageBox>
#include <QElapsedTimer>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkCookie>
#include <QEventLoop>
#include <QTimer>

#include <algorithm>

#include <zlib.h>

#include "dbmanager.h"
#include "randomnumber.h"
#include "httphandler.h"


class SThread : public QThread
{
    Q_OBJECT
public:
    SThread();
    SThread(QList<QString> fp);
    ~SThread();
    static bool checkIsRunning();
    static void setIsRunning(bool value);
    void setDbManager(DbManager *db);

signals:
    void sendData(const QString &d);
    void sendQAppquit();

public slots:
    void callTerminateThread();
    void receiveFromDbManager(const QString &data);

protected:
    void run();

private:
    QElapsedTimer timer;
    static QMutex mtx;
    static bool isRunning;
    const static qint64 max_last_visited_mseconds;

    bool isTryTerminate;
    QList<QString> fp;
    DbManager *db;
    RandomNumber *randnum;
    QList<QByteArray> cookies;
    QString extra_flags;

    void check_for_robotpage(bool &robotcheck, QByteArray tempfile, QString robot_regexp);
    bool libprocdata(QByteArray tempfile, QString hostname_base, QString currentUrl, QString current_time);
    bool testHTMLdoc(const QByteArray testfile);
    bool testIMG(const QByteArray testfile);

    QByteArray gUncompress(const QByteArray &data);
    QByteArray gCompress(const QByteArray &data);
};

#endif // STHREAD_H
