#include "sthread.h"

QMutex SThread::mtx;
bool SThread::isRunning;
const qint64 SThread::max_last_visited_mseconds = 60*60*24*7*1000;

SThread::SThread(QList<QString> fp){
    this->fp = fp;
    this->isTryTerminate = false;
    randnum = new RandomNumber();
}

SThread::SThread(){
    this->isTryTerminate = false;
    randnum = new RandomNumber();
}

SThread::~SThread(){
    mtx.unlock();
    this->quit();
    this->wait();
}

bool SThread::checkIsRunning(){
    return SThread::isRunning;
}

void SThread::setIsRunning(bool value){
    SThread::isRunning = value;
}

void SThread::callTerminateThread(){
    this->isTryTerminate = true;
}

void SThread::receiveFromDbManager(const QString &data){
    sendData(data);
}

void SThread::setDbManager(DbManager *db){
    this->db = db;
    connect(this->db, &DbManager::sendData, this, &SThread::receiveFromDbManager);
}

void SThread::run() Q_DECL_OVERRIDE{
    const double percent_chance_for_cookies_reset = 1.0;
    mtx.lock();
    if(this->isTryTerminate){
        mtx.unlock();
        return;
    }
    QString hostname_base = fp.at(0);
    QString default_url = fp.at(1);
    QString hostname_regexp = fp.at(2);
    QString random_wait = fp.at(3);
    extra_flags = fp.at(4);
    QString robot_regexp = fp.at(5);
    QString header = fp.at(6);
    QString current_time = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString currentUrl = default_url;
    QByteArray tempfile = "";
    long ret = -1;
    sendData("["+hostname_base+"] Thread Initialization...");

    int rowcount = db->nextUrlCount(QString::number(current_time.toULongLong()-max_last_visited_mseconds), hostname_regexp);
    if(rowcount > 0){
        int randRow = randnum->irand() % rowcount;
        sendData("["+hostname_base+"] RandRow/rowcount = "+QString::number(randRow)+"/"+QString::number(rowcount));
        currentUrl = db->nextUrl(QString::number(current_time.toULongLong()-max_last_visited_mseconds), hostname_regexp, randRow);
    }else if(rowcount == -1){
        return;
    }else{
        sendData("["+hostname_base+"] rowcount = 0, using default");
    }
    sendData("["+hostname_base+"] Thread Initialized. Next url = \""+currentUrl+"\"");
    sendData("ᐷ-------------------------------------------------------------ᐅ");
    mtx.unlock();
    if(this->isTryTerminate)
        return;
    QThread::msleep(100);
    while(true){
        mtx.lock();
        if(this->isTryTerminate){
            mtx.unlock();
            return;
        }
        sendData("["+hostname_base+"] wget "+'"'+currentUrl+'"');
        if( randnum->frand(0.0, 100.0) < percent_chance_for_cookies_reset ){
            sendData("["+hostname_base+"] Cookies have been randomly reset.");
            this->cookies.clear();
        }

        bool robotcheck = false;
        bool safebrowsingcheck = false;
        int timelock_release = 0;
        if(!robotcheck && !safebrowsingcheck){
            HTTPHandler *hc = new HTTPHandler();
            tempfile = hc->wget(QUrl(currentUrl), header, this->cookies);
            delete hc;

            if(this->isTryTerminate){
                mtx.unlock();
                return;
            }
            sendData("["+hostname_base+"] ...Finished downloading, got "+QString::number(tempfile.length())+" bytes");
            if(extra_flags.contains("robotcheck")){
                check_for_robotpage(robotcheck, tempfile, robot_regexp);
                if(robotcheck){
                    timelock_release = QDateTime::currentMSecsSinceEpoch()+(60*60*24*1000);
                    sendData("["+hostname_base+"] ROBOT! First instance for this trigger. timelock_release = "+QString::number(timelock_release));
                }
            }
            if(this->isTryTerminate){
                mtx.unlock();
                return;
            }
        }else if(robotcheck){
            sendData("["+hostname_base+"] ROBOTCHECK: skipping this lookup. timelock_release = "+QString::number(timelock_release));
        }else if(safebrowsingcheck){
            sendData("["+hostname_base+"] SAFEBROWSINGCHECK: skipping this lookup.");
        }
        if(this->isTryTerminate){
            mtx.unlock();
            return;
        }
        if(tempfile != ""){
            if(libprocdata(tempfile, hostname_base, currentUrl, current_time) == false){
                sendData("["+hostname_base+"] LibProcData Returned an Error");
                mtx.unlock();
                return;
            }
        }
        if(this->isTryTerminate){
            mtx.unlock();
            return;
        }

        int rowcount = db->nextUrlCount(QString::number(current_time.toULongLong()-max_last_visited_mseconds), hostname_regexp);
        if(rowcount > 0){
            int randRow = randnum->irand() % rowcount;
            sendData("["+hostname_base+"] RandRow/rowcount = "+QString::number(randRow)+"/"+QString::number(rowcount));
            currentUrl = db->nextUrl(QString::number(current_time.toULongLong()-max_last_visited_mseconds), hostname_regexp, randRow);
        }else if(rowcount == -1){
            mtx.unlock();
            return;
        }else{
            sendData("["+hostname_base+"] rowcount = 0, using default");
        }
        sendData("["+hostname_base+"] Thread setup. next url = \""+currentUrl+"\"");
        if(this->isTryTerminate){
            mtx.unlock();
            return;
        }

        double sleepsec = randnum->random_wait_parser(random_wait);
        if(sleepsec < 0){
            sleepsec = 30;
        }
        sendData("["+hostname_base+"] random sleep("+QString::number(sleepsec)+" sec)");
        sendData("ᐷ-------------------------------------------------------------ᐅ");
        mtx.unlock();
        QThread::msleep((int)(sleepsec*1000));
        if(this->isTryTerminate)
            return;
    }
}

void SThread::check_for_robotpage(bool &robotcheck, QByteArray tempfile, QString robot_regexp){
    if(QRegExp(robot_regexp).indexIn(tempfile, QRegExp::CaretAtZero) == -1){
        robotcheck = false;
    }else{
        robotcheck = true;
    }
}

bool SThread::testHTMLdoc(const QByteArray testfile){
    QByteArray tmp = testfile.mid(0,4096);
    if(
            QRegExp("\\<head\\>").indexIn(tmp, QRegExp::CaretAtZero) != -1 ||
            QRegExp("\\<head\\ ").indexIn(tmp, QRegExp::CaretAtZero) != -1 ||
            QRegExp("\\<title\\>").indexIn(tmp, QRegExp::CaretAtZero) != -1 ||
            QRegExp("\\<title\\ ").indexIn(tmp, QRegExp::CaretAtZero) != -1 ||
            QRegExp("\\<script\\>").indexIn(tmp, QRegExp::CaretAtZero) != -1 ||
            QRegExp("\\<script\\ ").indexIn(tmp, QRegExp::CaretAtZero) != -1 ||
            QRegExp("\\<style\\>").indexIn(tmp, QRegExp::CaretAtZero) != -1 ||
            QRegExp("\\<style\\ ").indexIn(tmp, QRegExp::CaretAtZero) != -1 ||
            QRegExp("\\<table\\>").indexIn(tmp, QRegExp::CaretAtZero) != -1 ||
            QRegExp("\\<table\\ ").indexIn(tmp, QRegExp::CaretAtZero) != -1 ||
            QRegExp("\\<a\\ href=").indexIn(tmp, QRegExp::CaretAtZero) != -1
    ){
        return true;
    }else{
        return false;
    }
}

bool SThread::testIMG(const QByteArray testfile){
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

bool SThread::libprocdata(QByteArray tempfile, QString hostname_base, QString currentUrl, QString current_time){
    sendData("["+hostname_base+"] data was retrieved and stored in \"tempfile\"");
    db->insertMetadata(currentUrl, current_time);
    double pagerank = db->getPagerankMetadata(currentUrl);
    if(pagerank < 1.0){
        sendData("Error: pagerank < 1.0, quitting...");
        sendQAppquit();
    }else{
        sendData("["+hostname_base+"] pagerank = "+QString::number(pagerank));
    }
    db->updateMetadata(currentUrl, current_time, pagerank);

    qlonglong id = db->getIdMetadata(currentUrl);
    sendData("["+hostname_base+"] contents id = " + QString::number(id));
    if(testHTMLdoc(tempfile)){
        sendData("["+hostname_base+"] HTML file, found");
        db->updateMagicMetadata(currentUrl, QByteArray("HTML document"));
        unsigned int count = db->extract_n_store_urls(tempfile, currentUrl, pagerank);
        sendData("["+hostname_base+"] href count = "+QString::number(count));
        if(extra_flags.contains("savekeywords")){
            sendData(db->extract_n_store_keywords(tempfile, id, hostname_base));
        }
        if(extra_flags.contains("saveallcontents")){
            sendData(db->extract_n_store_bodytext(tempfile, id, hostname_base));
        }
        if(extra_flags.contains("saveimgs")){
            sendData("["+hostname_base+"] Images, extracting...");
            QSet<QByteArray> image_hrefs = db->extract_images_hrefs(tempfile);
            db->store_images(image_hrefs, current_time, fp.at(6), cookies, max_last_visited_mseconds);
            sendData("["+hostname_base+"] Done extracting images.");
        }
    }else if(testIMG(tempfile)){
        db->updateMagicMetadata(currentUrl, QByteArray("Image File"));
        sendData("["+hostname_base+"] Image file, do not look for refs");
    }else{
        sendData("["+hostname_base+"] [-] not an HTML file, do not look for refs");
    }
    return true;
}

QByteArray SThread::gUncompress(const QByteArray &data){
    if (data.size() <= 4) {
        sendData("gUncompress: Input data is truncated");
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
    //ret = inflateInit2(&strm, 15 +  16);
    if(ret == Z_STREAM_ERROR){
        qDebug() << "Error: zlib: inflateInit2: Z_STREAM_ERROR";
        return QByteArray();
    }else if(ret != Z_OK){
        return QByteArray();
    }

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
            qDebug() << "Error: zlib: inflate: ret = " << ret;
            return QByteArray();
        }

        result.append(out, CHUNK_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);

    // clean up and return
    inflateEnd(&strm);
    return result;
}

QByteArray SThread::gCompress(const QByteArray &data){
    if (data.size() <= 4) {
        sendData("gUncompress: Input data is truncated");
        return QByteArray();
    }
    QByteArray result;

    int ret;
    z_stream strm;
    static const int CHUNK_SIZE = 1024;
    char out[CHUNK_SIZE];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = data.size();
    strm.next_in = (Bytef*)(data.data());

    //ret = inflateInit2(&strm, 15 +  32);
    //inflateInit2 (z_streamp strm, int  windowBits);
    ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY); // gzip encoding
    //ret = deflateInit2(z_streamp strm, int  level, int  method, int  windowBits, int  memLevel, int  strategy));
    if(ret == Z_STREAM_ERROR){
        qDebug() << "Error: zlib: deflateInit2: Z_STREAM_ERROR";
        return QByteArray();
    }else if(ret != Z_OK){
        return QByteArray();
    }
    // run deflate()
    /* do not write more than CHUNK bytes */
    do{
        strm.avail_out = CHUNK_SIZE;  /* maximum number of bytes to write */
        strm.next_out = (Bytef*)(out); /* pointer to output buffer */

        ret = deflate(&strm, Z_FINISH); /* no bad return value */
        Q_ASSERT(ret != Z_STREAM_ERROR);  // state not clobbered

        switch (ret) {
        case Z_NEED_DICT:
            ret = Z_DATA_ERROR;     // and fall through
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
            (void)deflateEnd(&strm);
            qDebug() << "Error: zlib: deflate: ret = " << ret;
            return QByteArray();
        }

        result.append(out, CHUNK_SIZE - strm.avail_out);
    } while (strm.avail_out == 0);

    // clean up and return
    deflateEnd(&strm);
    return result;
}
