#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QSqlDriver>

#include <htmlcxx/html/ParserDom.h>

#include "sqlite3.h"
#include "httphandler.h"

static void regexp(sqlite3_context *ctx, int argc, sqlite3_value **argv);

class DbManager: public QObject
{
    Q_OBJECT

signals:
    void sendData(const QString &d);
    void sendQAppquit();

public:
    DbManager(const QString& path);
    ~DbManager();
    bool isOpen() const;
    bool createTable();
    QByteArray getConfig(QByteArray name, QString type);
    void setConfig(QByteArray name, QString type, QVariant value);
    int nextUrlCount(QString last_visited, QString regexp);
    QByteArray nextUrl(QString last_visited, QString regexp, int randRow);
    void insertMetadata(QString url, QString last_visited);
    void insertMetadata(QString url, QString last_visited, QSqlQuery q);
    double getPagerankMetadata(QString url);
    double getPagerankMetadata(QString url, QSqlQuery queryAdd);
    void updateMetadata(QString url, QString last_visited, double pagerank);
    void updateMetadata(QString url, QString last_visited, double pagerank, QSqlQuery queryAdd);
    qlonglong getIdMetadata(QString url);
    qlonglong getIdMetadata(QString url, QSqlQuery q);
    qlonglong getTimestampMetadata(QString url);
    qlonglong getTimestampMetadata(QString url, QSqlQuery q);
    void updateMagicMetadata(QString url, QByteArray filetype);
    void updateMagicMetadata(QString url, QByteArray filetype, QSqlQuery q);
    unsigned int extract_n_store_urls(QByteArray tempfile, QString url, double pagerank);
    QString extract_n_store_keywords(QByteArray tempfile, int id, QString hostname_base);
    QSet<QByteArray> extract_images_hrefs(QByteArray tempfile);
    QString store_image(qlonglong id, QByteArray image_data, QString hostname_base, QSqlQuery queryAdd);
    QString store_images(QSet<QByteArray> image_hrefs, QString current_time, QString header, QList<QByteArray> cookies, qlonglong max_last_visited_mseconds);
    QString extract_n_store_bodytext(QByteArray tempfile, int id, QString hostname_base);
    QSqlQuery beginTrans();
    void commitTrans(QSqlQuery queryAdd);

private:
    QSqlDatabase m_QSqlD;
    QByteArray rel2abs_hyperlink(const QByteArray ref, const QByteArray currentUrl);
    QByteArray current_url_with_no_query_func(QByteArray currentUrl);
    QByteArray filter_entry(QByteArray tmp);
    QList<QByteArray> split_x(QByteArray qba, int size);
    bool testIMG(const QByteArray testfile);
};

#endif // DBMANAGER_H
