#include "dbmanager.h"

//http://doc.qt.io/qt-5/qtsql-index.html
//http://doc.qt.io/qt-5/sql-programming.html
//
//http://stackoverflow.com/questions/34415351/adding-a-custom-sqlite-function-to-a-qt-application

void regexp(sqlite3_context *ctx, int argc, sqlite3_value **argv){
    QRegExp regex;
    QString str1((const char*)sqlite3_value_text(argv[0]));
    QString str2((const char*)sqlite3_value_text(argv[1]));
    regex.setPattern(str1);
    if (str2.contains(regex)){
        sqlite3_result_int(ctx, 1);
    }else{
        sqlite3_result_int(ctx, 0);
    }
}

DbManager::DbManager(const QString &path)
{
    m_QSqlD = QSqlDatabase::addDatabase("QSQLITE");
    m_QSqlD.setDatabaseName(path);
    m_QSqlD.open();

    QVariant v = m_QSqlD.driver()->handle();
    if (v.isValid() && qstrcmp(v.typeName(), "sqlite3*")==0) {
        sqlite3 *db_handle = *static_cast<sqlite3 **>(v.data());
        if (db_handle != 0) { // check that it is not NULL
            sqlite3_initialize();
            sqlite3_create_function(db_handle, "REGEXP", 2, SQLITE_UTF8 | SQLITE_DETERMINISTIC, NULL, regexp, NULL, NULL);
        }
    }
}

DbManager::~DbManager()
{
    if (m_QSqlD.isOpen()){
        m_QSqlD.close();
    }
}

bool DbManager::isOpen() const{
    return m_QSqlD.isOpen();
}

bool DbManager::createTable(){
    QSqlQuery query;
    query.prepare("CREATE TABLE IF NOT EXISTS metadata(id INTEGER PRIMARY KEY, url TEXT UNIQUE NOT NULL, filetype TEXT, last_visited INTEGER, pagerank FLOAT);");
    if (!query.exec()){
        return false;
    }
    query.prepare("CREATE TABLE IF NOT EXISTS keywords(id INTEGER, sub_index INTEGER, data TEXT);");
    if (!query.exec()){
        return false;
    }
    query.prepare("CREATE TABLE IF NOT EXISTS contents(id INTEGER, sub_index INTEGER, data TEXT);");
    if (!query.exec()){
        return false;
    }
    query.prepare("CREATE TABLE IF NOT EXISTS safebrowsing(hostname TEXT UNIQUE NOT NULL, malware_flag INTEGER, timestamp INTEGER);");
    if (!query.exec()){
        return false;
    }
    query.prepare("CREATE TABLE IF NOT EXISTS config(name TEXT UNIQUE NOT NULL, bvalue BOOLEAN, ivalue INTEGER, fvalue FLOAT, tvalue TEXT);");
    if (!query.exec()){
        return false;
    }
    return true;
}

int DbManager::nextUrlCount(QString last_visited, QString regexp){
    QSqlQuery q(m_QSqlD);
    q.prepare("SELECT count(url) FROM metadata WHERE last_visited < :lv AND url REGEXP :regexp;");
    q.bindValue(":lv", last_visited.toULongLong());
    q.bindValue(":regexp", regexp.toLatin1(), QSql::In|QSql::Binary);

    if(q.exec() == false){
        qDebug() << "DbManager::nextUrlCount " << q.lastError().text();
        return -1;
    }

    q.next();
    int rowcount = q.value(0).toInt();
    return rowcount;
}

QByteArray DbManager::getConfig(QByteArray name, QString type){
    QSqlQuery q(m_QSqlD);
    int f = -1;
    if(type == "BOOLEAN"){
        q.prepare("SELECT bvalue FROM config WHERE name = :name;");
        q.bindValue(":name", name, QSql::In|QSql::Binary);
        if(q.exec() == false){
            qDebug() << "DbManager::getConfig BOOL " << q.lastError().text();
        }
        f = q.record().indexOf("bvalue");
    }else if(type == "INTEGER"){
        q.prepare("SELECT ivalue FROM config WHERE name = :name;");
        q.bindValue(":name", name, QSql::In|QSql::Binary);
        if(q.exec() == false){
            qDebug() << "DbManager::getConfig INT " << q.lastError().text();
        }
        f = q.record().indexOf("ivalue");
    }else if(type == "FLOAT"){
        q.prepare("SELECT fvalue FROM config WHERE name = :name;");
        q.bindValue(":name", name, QSql::In|QSql::Binary);
        if(q.exec() == false){
            qDebug() << "DbManager::getConfig FLOAT " << q.lastError().text();
        }
        f = q.record().indexOf("fvalue");
    }else if(type == "TEXT"){
        q.prepare("SELECT tvalue FROM config WHERE name = :name;");
        q.bindValue(":name", name, QSql::In|QSql::Binary);
        if(q.exec() == false){
            qDebug() << "DbManager::getConfig TEXT " << q.lastError().text();
        }
        f = q.record().indexOf("tvalue");
    }
    if(f!=-1 && q.next()){
        return q.value(f).toByteArray();
    }
    return NULL;
}

void DbManager::setConfig(QByteArray name, QString type, QVariant value){
    QSqlQuery q(m_QSqlD);
    if(value.isNull()){
        return;
    }else if(type == "BOOLEAN"){
        q.prepare("UPDATE OR IGNORE config SET bvalue = :bvalue WHERE name = :name;");
        q.bindValue(":name", name, QSql::In|QSql::Binary);
        q.bindValue(":bvalue", value.toBool());
        if(q.exec() == false){
            qDebug() << "DbManager::setConfig BOOL 1" << q.lastError().text();
        }
        q.prepare("INSERT OR IGNORE INTO config (name, bvalue) VALUES (:name, :bvalue);");
        q.bindValue(":name", name, QSql::In|QSql::Binary);
        q.bindValue(":bvalue", value.toBool());
        if(q.exec() == false){
            qDebug() << "DbManager::setConfig BOOL 2" << q.lastError().text();
        }
    }else if(type == "INTEGER"){
        q.prepare("UPDATE OR IGNORE config SET ivalue = :ivalue WHERE name = :name;");
        q.bindValue(":name", name, QSql::In|QSql::Binary);
        q.bindValue(":ivalue", value.toLongLong());
        if(q.exec() == false){
            qDebug() << "DbManager::setConfig INT " << q.lastError().text();
        }
        q.prepare("INSERT OR IGNORE INTO config (name, ivalue) VALUES (:name, :ivalue);");
        q.bindValue(":name", name, QSql::In|QSql::Binary);
        q.bindValue(":ivalue", value.toLongLong());
        if(q.exec() == false){
            qDebug() << "DbManager::setConfig INT " << q.lastError().text();
        }
    }else if(type == "FLOAT"){
        q.prepare("UPDATE OR IGNORE config SET fvalue = :fvalue WHERE name = :name;");
        q.bindValue(":name", name, QSql::In|QSql::Binary);
        q.bindValue(":fvalue", value.toDouble());
        if(q.exec() == false){
            qDebug() << "DbManager::setConfig FLOAT " << q.lastError().text();
        }
        q.prepare("INSERT OR IGNORE INTO config (name, fvalue) VALUES (:name, :fvalue);");
        q.bindValue(":name", name, QSql::In|QSql::Binary);
        q.bindValue(":fvalue", value.toDouble());
        if(q.exec() == false){
            qDebug() << "DbManager::setConfig FLOAT " << q.lastError().text();
        }
    }else if(type == "TEXT"){
        q.prepare("UPDATE OR IGNORE config SET tvalue = :tvalue WHERE name = :name;");
        q.bindValue(":name", name, QSql::In|QSql::Binary);
        q.bindValue(":tvalue", value.toByteArray(), QSql::In|QSql::Binary);
        if(q.exec() == false){
            qDebug() << "DbManager::setConfig TEXT " << q.lastError().text();
        }
        q.prepare("INSERT OR IGNORE INTO config (name, tvalue) VALUES (:name, :tvalue);");
        q.bindValue(":name", name, QSql::In|QSql::Binary);
        q.bindValue(":tvalue", value.toByteArray(), QSql::In|QSql::Binary);
        if(q.exec() == false){
            qDebug() << "DbManager::setConfig TEXT " << q.lastError().text();
        }
    }
}

QByteArray DbManager::nextUrl(QString last_visited, QString regexp, int randRow){
    QSqlQuery q(m_QSqlD);
    q.prepare("SELECT url FROM metadata WHERE last_visited < :lv AND url REGEXP :regexp LIMIT 1 OFFSET :randrow;");
    q.bindValue(":lv", last_visited.toLongLong());
    q.bindValue(":regexp", regexp.toLatin1(), QSql::In|QSql::Binary);
    q.bindValue(":randrow", randRow);

    if(q.exec() == false){
        qDebug() << "DbManager::nextUrl " << q.lastError().text();
        return QByteArray();
    }
    q.next();
    QSqlRecord rec = q.record();
    QByteArray url = q.value(rec.indexOf("url")).toByteArray();
    return url;
}

void DbManager::insertMetadata(QString url, QString last_visited){
    QSqlQuery queryAdd(m_QSqlD);
    insertMetadata(url, last_visited, queryAdd);
}

void DbManager::insertMetadata(QString url, QString last_visited, QSqlQuery queryAdd){
    queryAdd.prepare("INSERT OR IGNORE INTO metadata (url, last_visited) VALUES ((:url), (:last_visited));");
    queryAdd.bindValue(":url", url.toLatin1(), QSql::In|QSql::Binary);
    queryAdd.bindValue(":last_visited", last_visited.toLongLong());

    if(queryAdd.exec() == false){
        qDebug() << "DbManager::insertMetadata " << queryAdd.lastError().text();
    }
}

double DbManager::getPagerankMetadata(QString url){
    QSqlQuery queryAdd(m_QSqlD);
    return getPagerankMetadata(url, queryAdd);
}

double DbManager::getPagerankMetadata(QString url, QSqlQuery queryAdd){
    queryAdd.prepare("SELECT * FROM metadata WHERE url = :url;");
    queryAdd.bindValue(":url", url.toLatin1(), QSql::In|QSql::Binary);
    if(queryAdd.exec() == false){
        qDebug() << "DbManager::getPagerankMetadata " << queryAdd.lastError().text();
    }

    int fieldNo = queryAdd.record().indexOf("pagerank");
    if(queryAdd.next()){
        //qDebug() << queryAdd.value(fieldNo);
        double pagerank = queryAdd.value(fieldNo).toDouble();
        if(pagerank < 1.0)
            return 1.0;
        else
            return pagerank;
    }
    return 1.0;
}

void DbManager::updateMetadata(QString url, QString last_visited, double pagerank){
    QSqlQuery queryAdd(m_QSqlD);
    updateMetadata(url, last_visited, pagerank, queryAdd);
}

void DbManager::updateMetadata(QString url, QString last_visited, double pagerank, QSqlQuery queryAdd){
    queryAdd.prepare("UPDATE metadata SET last_visited = :last_visited, pagerank = :pagerank WHERE url = :url;");
    queryAdd.bindValue(":last_visited", last_visited.toLongLong());
    queryAdd.bindValue(":pagerank", pagerank);
    queryAdd.bindValue(":url", url.toLatin1(), QSql::In|QSql::Binary);
    if(queryAdd.exec() == false){
        qDebug() << "DbManager::updateMetadata " << queryAdd.lastError().text();
    }
}

qlonglong DbManager::getIdMetadata(QString url){
    QSqlQuery q(m_QSqlD);
    return getIdMetadata(url, q);
}

qlonglong DbManager::getIdMetadata(QString url, QSqlQuery q){
    if (!url.isEmpty()){
        q.prepare("SELECT id FROM metadata WHERE url = :url;");
        q.bindValue(":url", url.toLatin1(), QSql::In|QSql::Binary);

        if(q.exec() == false){
            qDebug() << "DbManager::getIdMetadata " << q.lastError().text();
            return -1;
        }
        q.next();
        QSqlRecord rec = q.record();
        qlonglong id = q.value(rec.indexOf("id")).toLongLong();
        return id;
    }else{
        return -1;
    }
}

qlonglong DbManager::getTimestampMetadata(QString url){
    QSqlQuery q(m_QSqlD);
    return getIdMetadata(url, q);
}

qlonglong DbManager::getTimestampMetadata(QString url, QSqlQuery q){
    if (!url.isEmpty()){
        q.prepare("SELECT last_visited FROM metadata WHERE url = :url;");
        q.bindValue(":url", url.toLatin1(), QSql::In|QSql::Binary);

        if(q.exec() == false){
            qDebug() << "DbManager::nextUrl- QSqlQuery " << q.lastError().text();
            return -1;
        }
        if(q.next()){
            QSqlRecord rec = q.record();
            qlonglong last_visited = q.value(rec.indexOf("last_visited")).toLongLong();
            return last_visited;
        }else{
            return -1;
        }
    }else{
        return -1;
    }
    return -1;
}

void DbManager::updateMagicMetadata(QString url, QByteArray filetype){
    QSqlQuery q(m_QSqlD);
    updateMagicMetadata(url, filetype, q);
}

void DbManager::updateMagicMetadata(QString url, QByteArray filetype, QSqlQuery q){
    if (!url.isEmpty()){
        q.prepare("UPDATE metadata SET filetype = :filetype WHERE url = :url;");
        q.bindValue(":url", url.toLatin1(), QSql::In|QSql::Binary);
        q.bindValue(":filetype", filetype, QSql::In|QSql::Binary);

        if(q.exec() == false){
            qDebug() << "DbManager::nextUrl- QSqlQuery " << q.lastError().text();
        }
    }
}

QSqlQuery DbManager::beginTrans(){
    QSqlQuery queryAdd(m_QSqlD);
    queryAdd.exec("BEGIN TRANSACTION");
    return queryAdd;
}

void DbManager::commitTrans(QSqlQuery queryAdd){
    queryAdd.exec("COMMIT");
}

unsigned int DbManager::extract_n_store_urls(QByteArray tempfile, QString url, double pagerank){
    tree<htmlcxx::HTML::Node> dom;
    htmlcxx::HTML::ParserDom parser;
    dom = parser.parseTree(tempfile.toStdString());
    tree<htmlcxx::HTML::Node>::iterator domit = dom.begin();
    tree<htmlcxx::HTML::Node>::iterator domend = dom.end();
    QByteArray tempurl;
    QSet<QByteArray> urlArray;
    for (; domit != domend; ++domit){
        if (strcasecmp(domit->tagName().c_str(), "A") == 0){
            domit->parseAttributes();
            tempurl = filter_entry(
                        rel2abs_hyperlink(
                            QString(domit->attribute("href").second.c_str()).toLatin1(), url.toLatin1()
                            )
                        ).data();
            QString qtempurl;
            int posh = tempurl.indexOf("#");
            if(posh == -1)
                qtempurl = QString(tempurl);
            else
                qtempurl = QString(tempurl.mid(0,posh));

            if(QRegExp("^$").indexIn(qtempurl.toLatin1(), QRegExp::CaretAtZero) == -1){
                if (!qtempurl.toLatin1().isEmpty()){
                    urlArray.insert(qtempurl.toLatin1());
                }
            }
        }
    }
    unsigned int count = urlArray.size();
    if(count != 0){
        QSqlQuery queryAdd(m_QSqlD);
        queryAdd.exec("BEGIN TRANSACTION");
        foreach (const QByteArray &value, urlArray){
            double oldpagerank = -1;
            queryAdd.prepare("SELECT pagerank FROM metadata WHERE url = :url;");
            queryAdd.bindValue(":url", value, QSql::In|QSql::Binary);
            if(queryAdd.exec() == false){
                qDebug() << "DbManager::extract_n_store_urls SELECT " << queryAdd.lastError().text();
            }

            int fieldNo = queryAdd.record().indexOf("pagerank");
            if(queryAdd.next()){
                oldpagerank = queryAdd.value(fieldNo).toDouble();
                queryAdd.prepare("UPDATE metadata SET pagerank = :pagerank WHERE url = :url;");
                queryAdd.bindValue(":url", value, QSql::In|QSql::Binary);
                queryAdd.bindValue(":pagerank", oldpagerank+(double)(pagerank/count));
                if(queryAdd.exec() == false){
                    qDebug() << "DbManager::extract_n_store_urls UPDATE " << queryAdd.lastError().text();
                }
            }else{
                queryAdd.prepare("INSERT INTO metadata(url, last_visited, pagerank) VALUES (:url, -1, :pagerank);");
                queryAdd.bindValue(":url", value, QSql::In|QSql::Binary);
                queryAdd.bindValue(":pagerank", 1.0+(double)(pagerank/count));
                if(queryAdd.exec() == false){
                    qDebug() << "DbManager::extract_n_store_urls INSERT " << queryAdd.lastError().text();
                }
            }
        }
        queryAdd.exec("COMMIT");
    }
    return count;

}

QString DbManager::extract_n_store_keywords(QByteArray tempfile, int id, QString hostname_base){
    tree<htmlcxx::HTML::Node> dom;
    htmlcxx::HTML::ParserDom parser;
    dom = parser.parseTree(tempfile.toStdString());
    tree<htmlcxx::HTML::Node>::iterator domit = dom.begin();
    tree<htmlcxx::HTML::Node>::iterator domend = dom.end();
    QByteArray temptextbody = QByteArray();
    bool accept = false;
    for (; domit != domend; ++domit){
        if(domit->isTag()){
            if(
                    strcasecmp(domit->tagName().c_str(), "SCRIPT") != 0 &&
                    strcasecmp(domit->tagName().c_str(), "STYLE") != 0 &&
                    strcasecmp(domit->tagName().c_str(), "META") != 0 &&
                    strcasecmp(domit->tagName().c_str(), "LINK") != 0
                    ){
                accept = true;
            }else{
                accept = false;
            }
            continue;
        }
        QByteArray temp = domit->text().c_str();
        if (accept && (!domit->isTag()) && (!domit->isComment()) && QRegExp("^\\s*$").indexIn(temp, QRegExp::CaretAtZero) == -1){
            if(QRegExp("<").indexIn(temp, QRegExp::CaretAtZero) != -1 || QRegExp(">").indexIn(temp, QRegExp::CaretAtZero) != -1){
                accept = false;
                continue;
            }
            temp = temp.replace(QByteArray("\\t"), "");
            temp = temp.replace(QByteArray("\\r"), "");
            temp = temp.replace(QByteArray("\\n"), "");
            int len;
            do{
                len = temp.size();
                temp = temp.replace(QByteArray("  "), " ");
            }while(len != temp.size());
            temptextbody += temp + " ";
            accept = false;
        }
    }
    QList<QByteArray> temptextbody_vec = this->split_x(temptextbody, 32768);
    QSqlQuery queryAdd(m_QSqlD);

    queryAdd.prepare("DELETE FROM keywords WHERE id = :id;");
    queryAdd.bindValue(":id", id);
    queryAdd.exec();

    queryAdd.exec("BEGIN TRANSACTION");
    QList<QString> result;
    for(int count = 0; count < temptextbody_vec.size(); count++){
        result.append("["+hostname_base+"] keywords store count (sizeof(QByte)*32768) = "+QString::number(count+1));
        queryAdd.prepare("INSERT INTO keywords(id, sub_index, data) VALUES ( :id , :count , :data );");
        queryAdd.bindValue(":id", id);
        queryAdd.bindValue(":count", count);
        queryAdd.bindValue(":data", temptextbody_vec.at(count));
        queryAdd.exec();
    }
    queryAdd.exec("COMMIT");
    return result.join('\n');
}

QSet<QByteArray> DbManager::extract_images_hrefs(QByteArray tempfile){
    tree<htmlcxx::HTML::Node> dom;
    htmlcxx::HTML::ParserDom parser;
    dom = parser.parseTree(tempfile.toStdString());
    tree<htmlcxx::HTML::Node>::iterator domit = dom.begin();
    tree<htmlcxx::HTML::Node>::iterator domend = dom.end();
    QSet<QByteArray> image_hrefs;
    for (; domit != domend; ++domit){
        if(domit->isTag()){
            if(
                    strcasecmp(domit->tagName().c_str(), "IMG") == 0
                    ){
                domit->parseAttributes();
                image_hrefs.insert(QByteArray(domit->attribute("src").second.c_str()));
            }
        }
    }
    return image_hrefs;
}

bool DbManager::testIMG(const QByteArray testfile){
    QByteArray tmp = testfile.mid(0,4096);
    if(
            tmp.mid(0,4) == QByteArray("\x47\x49\x46\x38", 4) || //GIF
            tmp.mid(0,2) == QByteArray("\xff\xd8", 2) || //JPEG
            tmp.mid(0,2) == QByteArray("\x42\x4d", 2) || //BMP
            tmp.mid(0,9) == QByteArray("\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x20", 9) //PNG
    ){
        return true;
    }else{
        return false;
    }
}

QString DbManager::store_images(QSet<QByteArray> image_hrefs, QString current_time, QString header, QList<QByteArray> cookies, qlonglong max_last_visited_mseconds){
    QSqlQuery queryAdd(m_QSqlD);
    queryAdd.exec("BEGIN TRANSACTION");
    QList<bool> isDownloadThisImage;
    int i = 0;
    foreach(QByteArray url, image_hrefs){
        if(!QUrl(url).isValid()){
            continue;
        }
        qlonglong lastv = -1;
        queryAdd.prepare("SELECT last_visited FROM metadata WHERE url = :url;");
        queryAdd.bindValue(":url", url, QSql::In|QSql::Binary);
        if(queryAdd.exec() == false){
            qDebug() << "DbManager::nextUrl- QSqlQuery " << queryAdd.lastError().text();
        }else{
            if(queryAdd.next()){
                QSqlRecord rec = queryAdd.record();
                lastv = queryAdd.value(rec.indexOf("last_visited")).toLongLong();
            }
        }
        if(lastv < current_time.toLongLong() - max_last_visited_mseconds){
            isDownloadThisImage.append(true);
        }else{
            isDownloadThisImage.append(false);
        }
        i++;
    }
    QList<QByteArray> imageBlob;
    i = 0;
    foreach(QByteArray url, image_hrefs){
        if(isDownloadThisImage.at(i)){
            HTTPHandler *hc = new HTTPHandler();
            //hc->setMagic(magic);
            QByteArray image_data = hc->wget(QUrl(url), header, cookies);
            delete hc;
            imageBlob.append(image_data);
        }
        i++;
    }
    i = 0;
    foreach(QByteArray url, image_hrefs){
        if(isDownloadThisImage.at(i)){
            QByteArray image_data = imageBlob.at(i);
            if(testIMG(image_data)){
                queryAdd.prepare("INSERT OR IGNORE INTO metadata (url, last_visited) VALUES ((:url), (:last_visited));");
                queryAdd.bindValue(":url", url, QSql::In|QSql::Binary);
                queryAdd.bindValue(":last_visited", current_time.toLongLong());

                if(queryAdd.exec() == false){
                    qDebug() << "DbManager::store_images, insertMetadata " << queryAdd.lastError().text();
                }

                double pagerank = 1.0;
                queryAdd.prepare("SELECT * FROM metadata WHERE url = :url;");
                queryAdd.bindValue(":url", url, QSql::In|QSql::Binary);
                if(queryAdd.exec() == false){
                    qDebug() << "DbManager::store_images, getPagerankMetadata " << queryAdd.lastError().text();
                }

                int fieldNo = queryAdd.record().indexOf("pagerank");
                if(queryAdd.next()){
                    pagerank = queryAdd.value(fieldNo).toDouble();
                    if(pagerank < 1.0)
                        pagerank = 1.0;
                }else
                    pagerank = 1.0;

                queryAdd.prepare("UPDATE metadata SET last_visited = :last_visited, pagerank = :pagerank WHERE url = :url;");
                queryAdd.bindValue(":last_visited", current_time.toLongLong());
                queryAdd.bindValue(":pagerank", pagerank);
                queryAdd.bindValue(":url", url, QSql::In|QSql::Binary);
                if(queryAdd.exec() == false){
                    qDebug() << "DbManager::store_images, updateMetadata " << queryAdd.lastError().text();
                }

                qlonglong img_id = -1;
                queryAdd.prepare("SELECT id FROM metadata WHERE url = :url;");
                queryAdd.bindValue(":url", url, QSql::In|QSql::Binary);

                if(queryAdd.exec() == false){
                    qDebug() << "DbManager::getIdMetadata " << queryAdd.lastError().text();
                }else{
                    if(queryAdd.next()){
                        QSqlRecord rec = queryAdd.record();
                        qlonglong id = queryAdd.value(rec.indexOf("id")).toLongLong();
                        img_id = id;
                    }
                }
                queryAdd.prepare("UPDATE metadata SET filetype = :filetype WHERE url = :url;");
                queryAdd.bindValue(":url", url, QSql::In|QSql::Binary);
                queryAdd.bindValue(":filetype", QByteArray("Image File"), QSql::In|QSql::Binary);

                if(queryAdd.exec() == false){
                    qDebug() << "DbManager::nextUrl- QSqlQuery " << queryAdd.lastError().text();
                }

                queryAdd.prepare("DELETE FROM contents WHERE id = :id;");
                queryAdd.bindValue(":id", img_id);
                queryAdd.exec();

                queryAdd.prepare("INSERT INTO contents(id, sub_index, data) VALUES ( :id , :count , :data );");
                queryAdd.bindValue(":id", img_id);
                queryAdd.bindValue(":count", 0);
                queryAdd.bindValue(":data", image_data);
                queryAdd.exec();
            }
        }
        i++;
    }
    queryAdd.exec("COMMIT");
    return QString();
}

QString DbManager::store_image(qlonglong id, QByteArray image_data, QString hostname_base, QSqlQuery queryAdd){
    queryAdd.prepare("DELETE FROM contents WHERE id = :id;");
    queryAdd.bindValue(":id", id);
    queryAdd.exec();

    QList<QString> result;
    queryAdd.prepare("INSERT INTO contents(id, sub_index, data) VALUES ( :id , :count , :data );");
    queryAdd.bindValue(":id", id);
    queryAdd.bindValue(":count", 0);
    queryAdd.bindValue(":data", image_data);
    queryAdd.exec();
    return result.join('\n');
}

QString DbManager::extract_n_store_bodytext(QByteArray tempfile, int id, QString hostname_base){
    QList<QByteArray> temptextbody_vec = this->split_x(tempfile, 32768);
    QSqlQuery queryAdd(m_QSqlD);

    queryAdd.prepare("DELETE FROM contents WHERE id = :id;");
    queryAdd.bindValue(":id", id);
    queryAdd.exec();

    queryAdd.exec("BEGIN TRANSACTION");
    QList<QString> result;
    for(int count = 0; count < temptextbody_vec.size(); count++){
        result.append("["+hostname_base+"] contents store count (sizeof(QByte)*32768) = "+QString::number(count+1));
        queryAdd.prepare("INSERT INTO contents(id, sub_index, data) VALUES ( :id , :count , :data );");
        queryAdd.bindValue(":id", id);
        queryAdd.bindValue(":count", count);
        queryAdd.bindValue(":data", temptextbody_vec.at(count));
        queryAdd.exec();
    }
    queryAdd.exec("COMMIT");
    return result.join('\n');
}

QByteArray DbManager::rel2abs_hyperlink(const QByteArray ref, const QByteArray currentUrl){
    QByteArray newref = "";
    if(QRegExp("^#").indexIn(ref, QRegExp::CaretAtZero) != -1 || QRegExp("^$").indexIn(ref, QRegExp::CaretAtZero) != -1){
        return "";
    }else if(QRegExp("^[a-z]+://").indexIn(ref, QRegExp::CaretAtZero) != -1){
        return newref; // probably a full link? might be bad
    }else if(QRegExp("^//").indexIn(ref, QRegExp::CaretAtZero) != -1){
        QString proto = currentUrl;
        proto.replace(QRegExp("^([^:]+)://.*$"), "\\1");
        return proto.toLatin1() + QByteArray(":") + ref;
    }else if(QRegExp("^/[^/]").indexIn(ref, QRegExp::CaretAtZero) != -1 || QRegExp("^/$").indexIn(ref, QRegExp::CaretAtZero) != -1){
        QString current_url_proto_hostname = current_url_with_no_query_func(currentUrl);
        current_url_proto_hostname.replace(QRegExp("^([^:]+://[^/]+).*$"), "\\1");
        return current_url_proto_hostname.toLatin1() + ref;
    }else if(QRegExp("^[^/]").indexIn(ref, QRegExp::CaretAtZero) != -1){
        QByteArray current_url_with_no_query = current_url_with_no_query_func(currentUrl);
        QString current_url_noproto_nohostname = current_url_with_no_query;
        current_url_noproto_nohostname.replace(QRegExp("^[^:]+://[^/]+(.*)$"), "\\1");
        int slashpos = current_url_noproto_nohostname.lastIndexOf("/");
        QString current_url_last_entry;
        if(slashpos != -1){
            current_url_last_entry = current_url_noproto_nohostname.mid(slashpos+1);
        }else{
            current_url_last_entry = current_url_noproto_nohostname;
        }
        int dot_index = current_url_last_entry.lastIndexOf(".");
        if(dot_index != -1){
            if(QRegExp("/$").indexIn(current_url_with_no_query, QRegExp::CaretAtZero) == -1){
                return current_url_with_no_query + "/" + ref;
            }else{
                return current_url_with_no_query + ref;
            }
        }else{
            QString current_url_strip_last_filename = current_url_with_no_query.mid(current_url_with_no_query.lastIndexOf("/")+1);
            if(QRegExp("/$").indexIn(current_url_strip_last_filename, QRegExp::CaretAtZero) == -1){
                return current_url_strip_last_filename.toLatin1() + "/" + ref;
            }else{
                return current_url_strip_last_filename.toLatin1() + ref;
            }
        }
    }else{
        qDebug() << "Error probabley with regexp's";
        qDebug() << "ref: \"" << ref << "\"";
        qDebug() << "current_url: \"" << currentUrl << "\"";
        exit(1);
        return "";
    }
}

QByteArray DbManager::current_url_with_no_query_func(QByteArray currentUrl){
    QByteArray current_url_with_no_query;
    int breaker_index;
    int breaker_index_q = currentUrl.indexOf("?");
    int breaker_index_h = currentUrl.indexOf("#");

    if(breaker_index_q == -1 && breaker_index_h == -1){
        breaker_index = -1;
    }else if(breaker_index_q != -1 && breaker_index_h == -1){
        breaker_index = breaker_index_q;
    }else if(breaker_index_q == -1 && breaker_index_h != -1){
        breaker_index = breaker_index_h;
    }else if(breaker_index_q <= breaker_index_h){
        breaker_index = breaker_index_q;
    }else{
        breaker_index = breaker_index_h;
    }

    if(breaker_index != -1){
        current_url_with_no_query = currentUrl.mid(0,breaker_index);
    }else{
        current_url_with_no_query = currentUrl;
    }
    return current_url_with_no_query;
}

QByteArray DbManager::filter_entry(QByteArray tmp){
    QString str = tmp;
    if(QRegExp("'").indexIn(str, QRegExp::CaretAtZero) != -1){
        return "";
    }else if(QRegExp("\"").indexIn(str, QRegExp::CaretAtZero) != -1){
        return "";
    }else if(QRegExp("^https?://[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}").indexIn(str, QRegExp::CaretAtZero) != -1){
        return "";
    }else
        return tmp;
}

QList<QByteArray> DbManager::split_x(QByteArray qba, int size){
    int quot = qba.length() / size;
    int rem = qba.length() % size;
    QList<QByteArray> qlqba;
    for(int i = 0; i < quot; i++){
        qlqba.append(qba.mid(i*size, size));
    }
    if(rem != 0)
        qlqba.append(qba.mid(size*quot));
    return qlqba;
}
