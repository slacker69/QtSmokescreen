#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QSystemTrayIcon>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QtDBus>
#include <QThread>
#include <QMutex>
#include <QtSql>
#include <QFileDialog>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "addthreaddialog.h"
#include "sthread.h"
#include "dbmanager.h"
#include "httphandler.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void on_pushButton_plus_clicked();
    void on_pushButton_edit_clicked();
    void on_pushButton_minus_clicked();
    void on_pushButton_start_clicked();
    void on_pushButton_stop_clicked();
    void on_actionOpen_triggered();
    void on_actionSave_As_triggered();
    void on_actionAutostart_toggled();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();

signals:
    void callTerminateThread();

public slots:
    void receiveData(QString s);
    void qAppquit();

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setVisible(bool visible);

private:
    Ui::MainWindow *ui;
    void printLogo();
    void actionOpenByteArray(QByteArray qba);
    QByteArray actionSaveasByteArray();
    bool firstRun;

    QAction *minimizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    AddThreadDialog *atd;
    QList<QList<QString> > fps;
    QList<QThread*> *threadList;
    DbManager *m_db;
    struct magic_set *magic;
};

#endif // MAINWINDOW_H
