#ifndef ADDTHREADDIALOG_H
#define ADDTHREADDIALOG_H

#include <QDialog>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QPicture>
#include <QLabel>

#include "randomnumber.h"

namespace Ui {
class AddThreadDialog;
}

class AddThreadDialog : public QDialog
{
    Q_OBJECT
private slots:
    void on_checkBox_robotcheck_stateChanged(int state);
    void on_comboBox_spread_currentIndexChanged(const QString &text);
    void on_pushButton_test_clicked();
    void on_pushButton_clearsettings_clicked();
    void on_pushButton_add_clicked();
    void on_pushButton_cancel_clicked();

public:
    explicit AddThreadDialog(QWidget *parent = 0);
    ~AddThreadDialog();
    bool getATDactive();
    QList<QString> getData();
    void loadData(QStringList fp);
    void setATDactive(bool state);
    void setButtonName1(QString name);
    void setButtonName2(QString name);
private:
    Ui::AddThreadDialog *ui;
    bool isATDactive;
    QList<QString> data;
    QFrame *popup;
    QLabel *l;

};

#endif // ADDTHREADDIALOG_H
