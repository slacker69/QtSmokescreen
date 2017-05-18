#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    minimizeAction = new QAction(tr("&Minimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
    restoreAction = new QAction(tr("&Show"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
    quitAction = new QAction(tr("&Exit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addAction(quitAction);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    QIcon icon = QIcon(":/images/smokescreen.png");
    trayIcon->setIcon(icon);
    setWindowIcon(icon);
    trayIcon->show();
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    atd = new AddThreadDialog();
    threadList = new QList<QThread*>;
    m_db = new DbManager("qss.db");
    if(!m_db->createTable()){
        qApp->quit();
    }
    connect(m_db, &DbManager::sendQAppquit, this, &MainWindow::qAppquit);

    if(m_db->getConfig(QByteArray("autostart"), "BOOLEAN") == NULL){
        m_db->setConfig(QByteArray("autostart"), "BOOLEAN", false);
    }
    if(m_db->getConfig(QByteArray("autostart"), "BOOLEAN") == "1")
        ui->actionAutostart->setChecked(true);
    else
        ui->actionAutostart->setChecked(false);


    QFont font("Monospace");
    font.setStyleHint(QFont::TypeWriter);
    ui->plainTextEdit->setMaximumBlockCount(200);
    ui->plainTextEdit->setFont(font);
    ui->plainTextEdit->setStyleSheet("QPlainTextEdit { background-color : black; color : green; }");
    this->setStyleSheet("QMainWindow {border-image: url(:/images/Background.png);}");

    printLogo();
    firstRun = true;
    if(m_db->getConfig(QByteArray("fps"), "TEXT") != NULL){
        actionOpenByteArray(m_db->getConfig(QByteArray("fps"), "TEXT"));
    }
    if(ui->actionAutostart->isChecked()){
        on_pushButton_start_clicked();
    }
}

MainWindow::~MainWindow(){
    delete ui;
    delete threadList;
    delete m_db;
}

void MainWindow::printLogo(){
    receiveData("  ____  __    ____           __                              ");
    receiveData(" / __ \\/ /_  / __/_ _  ___  / /_____ ___ ___________ ___ ___ ");
    receiveData("/ /_/ / __/ _\\ \\/  ' \\/ _ \\/  '_/ -_|_-</ __/ __/ -_) -_) _ \\");
    receiveData("\\___\\_\\__/ /___/_/_/_/\\___/_/\\_\\\\__/___/\\__/_/  \\__/\\__/_//_/");
    receiveData("                                                             ");
}

void MainWindow::receiveData(QString s){
    ui->plainTextEdit->appendPlainText(s);
    qDebug() << s;
}


void MainWindow::qAppquit(){
    QTimer::singleShot(250, qApp, SLOT(quit()));
}

void MainWindow::setVisible(bool visible){
    minimizeAction->setEnabled(visible);
    restoreAction->setEnabled(isMaximized() || !visible);
    QMainWindow::setVisible(visible);
    if(atd->getATDactive()){
        if(visible)
            atd->show();
        else{
            atd->hide();
            atd->setATDactive(false);
        }
    }
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason){
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        if(isVisible())
            setVisible(false);
        else
            setVisible(true);
        break;
    default:
        ;
    }
}

void MainWindow::on_pushButton_start_clicked(){
    if(!SThread::checkIsRunning() && fps.length() != 0){
        if(!firstRun)
            printLogo();
        firstRun = false;
        receiveData("Started...");
        SThread::setIsRunning(true);
        for(int i = 0; i < fps.length(); i++){
            SThread *s = new SThread(fps.at(i));
            threadList->append(s);
            connect(s, &SThread::sendData, this, &MainWindow::receiveData, Qt::QueuedConnection);
            connect(this, &MainWindow::callTerminateThread, s, &SThread::callTerminateThread, Qt::QueuedConnection);
            connect(s, &SThread::sendQAppquit, this, &MainWindow::qAppquit, Qt::QueuedConnection);
            s->setDbManager(m_db);
            s->start();
        }
    }
}

void MainWindow::on_pushButton_stop_clicked(){
    if(SThread::checkIsRunning()){
        callTerminateThread();
        receiveData("...Stopped");
        threadList->clear();
    }
    SThread::setIsRunning(false);
}

void MainWindow::on_pushButton_plus_clicked(){
    if(!SThread::checkIsRunning()){
        atd->setATDactive(true);
        atd->setButtonName1("Add");
        atd->setButtonName2("Cancel");
        atd->show();
        atd->exec();
        QList<QString> tmp = atd->getData();
        if(tmp.size() > 0){
            ui->comboBox->addItem(tmp.at(0));
            fps.append(tmp);
        }
        m_db->setConfig(QByteArray("fps"), "TEXT", actionSaveasByteArray());
    }
}

void MainWindow::on_pushButton_edit_clicked(){
    if(!SThread::checkIsRunning()){
        int place = ui->comboBox->currentIndex();
        if(place == -1)
            return;
        atd->loadData(fps.at(place));
        ui->comboBox->removeItem(place);
        fps.removeAt(place);
        atd->setATDactive(true);
        atd->setButtonName1("Re-Save");
        atd->setButtonName2("Remove");
        atd->show();
        atd->exec();
        QList<QString> tmp = atd->getData();
        if(tmp.size() > 0){
            ui->comboBox->addItem(tmp.at(0));
            fps.append(tmp);
        }
        m_db->setConfig(QByteArray("fps"), "TEXT", actionSaveasByteArray());
    }
}

void MainWindow::on_pushButton_minus_clicked(){
    if(!SThread::checkIsRunning()){
        int place = ui->comboBox->currentIndex();
        if(place == -1)
            return;
        ui->comboBox->removeItem(place);
        fps.removeAt(place);
        m_db->setConfig(QByteArray("fps"), "TEXT", actionSaveasByteArray());
    }
}

void MainWindow::on_actionOpen_triggered(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), ".");
    if(!fileName.isEmpty() && !fileName.isNull()){
        ui->comboBox->clear();
        fps.clear();
        QFile pFile(fileName);
        pFile.open(QIODevice::ReadOnly);
        QByteArray allbytes = pFile.readAll();
        actionOpenByteArray(allbytes);
        pFile.close();
        m_db->setConfig(QByteArray("fps"), "TEXT", allbytes);
    }
}

void MainWindow::actionOpenByteArray(QByteArray qba){
    QList<QByteArray> tmp1 = qba.split('\xFF');
    if(tmp1.last().isEmpty())
        tmp1.removeLast();
    for(int i = 0; i < tmp1.length(); i++){
        QList<QByteArray> tmp2 = tmp1.at(i).split('\x00');
        QList<QString> tmp3;
        for(int j = 0; j < tmp2.length(); j++){
            tmp3.append(QString(tmp2.at(j)));
        }
        ui->comboBox->addItem(tmp3.at(0));
        fps.append(tmp3);
    }
}

void MainWindow::on_actionSave_As_triggered(){
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), ".");
    if(!fileName.isEmpty() && !fileName.isNull()){
        QFile pFile(fileName);
        pFile.open(QIODevice::WriteOnly|QIODevice::Truncate);
        QByteArray qba = actionSaveasByteArray();
        pFile.write(qba, qba.size());
        pFile.close();
        m_db->setConfig(QByteArray("fps"), "TEXT", actionSaveasByteArray());
    }
}

QByteArray MainWindow::actionSaveasByteArray(){
    QByteArray result = QByteArray();
    for(int i = 0; i < fps.length(); i++){
        for(int j = 0; j < fps.at(i).length(); j++){
            result += QByteArray(fps.at(i).at(j).toLatin1()+"\x00", fps.at(i).at(j).length()+1);
        }
        result += QByteArray("\xFF", 1);
    }
    return result;
}

void MainWindow::on_actionAutostart_toggled(){
    if(ui->actionAutostart->isChecked()){
        m_db->setConfig(QByteArray("autostart"), "BOOLEAN", true);
    }else{
        m_db->setConfig(QByteArray("autostart"), "BOOLEAN", false);
    }
    m_db->setConfig(QByteArray("fps"), "TEXT", actionSaveasByteArray());
}

void MainWindow::on_actionExit_triggered(){
    m_db->setConfig(QByteArray("fps"), "TEXT", actionSaveasByteArray());
    QTimer::singleShot(250, qApp, SLOT(quit()));
}

void MainWindow::on_actionAbout_triggered(){
    QMessageBox::information(this, tr("QtSmokescreen"), tr("QtSmokescreen v0.1.4\n\tCreated By Fugu\n")+
                             tr("This application is a webscrapper, able to save web resources into an\n")+
                             tr("sqlite database, and will automattically surf and follow new url links,\n")+
                             tr("will do so in a highly random nature."), QMessageBox::Ok);
}
