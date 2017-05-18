#include "addthreaddialog.h"
#include "ui_addthreaddialog.h"

AddThreadDialog::AddThreadDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddThreadDialog)
{
    ui->setupUi(this);
    isATDactive = false;
    this->setStyleSheet("AddThreadDialog {border-image: url(:/images/Background.png);}");
    QString styleSheetLabelATD = "QLabel {color: white; font-weight: bold; text-decoration: underline;}";
    QString styleSheetCheckBoxATD = "QCheckBox {border: none; color: white; font-weight: bold; text-decoration: underline;}";
    ui->label_regex->setStyleSheet(styleSheetLabelATD);
    ui->label_initpage->setStyleSheet(styleSheetLabelATD);
    ui->label_title->setStyleSheet(styleSheetLabelATD);
    ui->label_minsec->setStyleSheet(styleSheetLabelATD);
    ui->label_maxsec->setStyleSheet(styleSheetLabelATD);
    ui->label_spread->setStyleSheet(styleSheetLabelATD);
    ui->label_header->setStyleSheet(styleSheetLabelATD);
    ui->label_mean->setStyleSheet(styleSheetLabelATD);
    ui->label_sdev->setStyleSheet(styleSheetLabelATD);
    ui->label_gamma->setStyleSheet(styleSheetLabelATD);
    ui->label_a->setStyleSheet(styleSheetLabelATD);
    ui->checkBox_robotcheck->setStyleSheet(styleSheetCheckBoxATD);
    ui->checkBox_safebrowsing->setStyleSheet(styleSheetCheckBoxATD);
    ui->checkBox_saveallcontents->setStyleSheet(styleSheetCheckBoxATD);
    ui->checkBox_saveimgs->setStyleSheet(styleSheetCheckBoxATD);
    ui->checkBox_savekeywords->setStyleSheet(styleSheetCheckBoxATD);
    ui->label_mean->hide();
    ui->doubleSpinBox_mean->hide();
    ui->label_sdev->hide();
    ui->doubleSpinBox_sdev->hide();
    ui->label_gamma->hide();
    ui->doubleSpinBox_gamma->hide();
    ui->label_a->hide();
    ui->doubleSpinBox_a->hide();


    popup = new QFrame(this, Qt::Window );
    popup->setWindowTitle(tr("Random Spread"));
    l = new QLabel(popup);

}

AddThreadDialog::~AddThreadDialog()
{
    delete ui;
}

QList<QString> AddThreadDialog::getData(){
    QList<QString> tmp = data;
    data.clear();
    return tmp;
}

void AddThreadDialog::setButtonName1(QString name){
    ui->pushButton_add->setText(name);

}

void AddThreadDialog::setButtonName2(QString name){
    ui->pushButton_cancel->setText(name);

}

void AddThreadDialog::loadData(QStringList fp){
    ui->lineEdit_title->setText(fp.at(0));
    ui->lineEdit_initialpage->setText(fp.at(1));
    ui->lineEdit_regex->setText(fp.at(2));
    QStringList tmp = fp.at(3).split(",");
    if(tmp.at(0).toStdString() == "Constant"){
        ui->comboBox_spread->setCurrentIndex(0);
    }else if(tmp.at(0).toStdString() == "Gauss"){
        ui->comboBox_spread->setCurrentIndex(1);
        ui->doubleSpinBox_mean->setValue(tmp.at(3).toDouble());
        ui->doubleSpinBox_sdev->setValue(tmp.at(4).toDouble());
    }else if(tmp.at(0).toStdString() == "Cauchy"){
        ui->comboBox_spread->setCurrentIndex(2);
        ui->doubleSpinBox_mean->setValue(tmp.at(3).toDouble());
        ui->doubleSpinBox_gamma->setValue(tmp.at(4).toDouble());
    }else if(tmp.at(0).toStdString() == "MaxBoltz"){
        ui->comboBox_spread->setCurrentIndex(3);
        ui->doubleSpinBox_a->setValue(tmp.at(3).toDouble());
    }
    ui->doubleSpinBox_spreadmin->setValue(tmp.at(1).toDouble());
    ui->doubleSpinBox_spreadmax->setValue(tmp.at(2).toDouble());
    tmp = fp.at(4).split(",");
    for(int i = 0; i < tmp.length(); i++){
        if(tmp.at(i).toStdString() == "safebrowsing")
            ui->checkBox_safebrowsing->setChecked(true);
        if(tmp.at(i).toStdString() == "saveallcontents")
            ui->checkBox_saveallcontents->setChecked(true);
        if(tmp.at(i).toStdString() == "savekeywords")
            ui->checkBox_savekeywords->setChecked(true);
        if(tmp.at(i).toStdString() == "saveimgs")
            ui->checkBox_saveimgs->setChecked(true);
        if(tmp.at(i).toStdString() == "robotcheck")
            ui->checkBox_robotcheck->setChecked(true);
    }
    ui->lineEdit_robotcheck->setText(fp.at(5));
    ui->plainTextEdit_header->setPlainText(fp.at(6));
}


bool AddThreadDialog::getATDactive(){
    return isATDactive;
}

void AddThreadDialog::setATDactive(bool state){
    isATDactive = state;
}

void AddThreadDialog::on_comboBox_spread_currentIndexChanged(const QString &text){
    if(text == "Constant"){
        ui->label_mean->hide();
        ui->doubleSpinBox_mean->hide();
        ui->label_sdev->hide();
        ui->doubleSpinBox_sdev->hide();

        ui->label_gamma->hide();
        ui->doubleSpinBox_gamma->hide();

        ui->label_a->hide();
        ui->doubleSpinBox_a->hide();
    }else if(text == "Gauss"){
        ui->label_mean->show();
        ui->doubleSpinBox_mean->show();
        ui->label_sdev->show();
        ui->doubleSpinBox_sdev->show();

        ui->label_gamma->hide();
        ui->doubleSpinBox_gamma->hide();

        ui->label_a->hide();
        ui->doubleSpinBox_a->hide();
    }else if(text == "Cauchy"){
        ui->label_mean->hide();
        ui->doubleSpinBox_mean->hide();
        ui->label_sdev->hide();
        ui->doubleSpinBox_sdev->hide();

        ui->label_mean->show();
        ui->doubleSpinBox_mean->show();
        ui->label_gamma->show();
        ui->doubleSpinBox_gamma->show();

        ui->label_a->hide();
        ui->doubleSpinBox_a->hide();
    }else if(text == "MaxBoltz"){
        ui->label_mean->hide();
        ui->doubleSpinBox_mean->hide();
        ui->label_sdev->hide();
        ui->doubleSpinBox_sdev->hide();

        ui->label_gamma->hide();
        ui->doubleSpinBox_gamma->hide();

        ui->label_a->show();
        ui->doubleSpinBox_a->show();
    }
}


void AddThreadDialog::on_checkBox_robotcheck_stateChanged(int state){
    ui->lineEdit_robotcheck->setEnabled(state);
}

void AddThreadDialog::on_pushButton_test_clicked(){
    QStringList spread;
    if(ui->comboBox_spread->currentText() == "Constant"){
        spread.append(ui->comboBox_spread->currentText());
        spread.append(QString::number(ui->doubleSpinBox_spreadmin->value()));
        spread.append(QString::number(ui->doubleSpinBox_spreadmax->value()));
    }else if(ui->comboBox_spread->currentText() == "Gauss"){
        spread.append(ui->comboBox_spread->currentText());
        spread.append(QString::number(ui->doubleSpinBox_spreadmin->value()));
        spread.append(QString::number(ui->doubleSpinBox_spreadmax->value()));
        spread.append(QString::number(ui->doubleSpinBox_mean->value()));
        spread.append(QString::number(ui->doubleSpinBox_sdev->value()));
    }else if(ui->comboBox_spread->currentText() == "Cauchy"){
        spread.append(ui->comboBox_spread->currentText());
        spread.append(QString::number(ui->doubleSpinBox_spreadmin->value()));
        spread.append(QString::number(ui->doubleSpinBox_spreadmax->value()));
        spread.append(QString::number(ui->doubleSpinBox_mean->value()));
        spread.append(QString::number(ui->doubleSpinBox_gamma->value()));
    }else if(ui->comboBox_spread->currentText() == "MaxBoltz"){
        spread.append(ui->comboBox_spread->currentText());
        spread.append(QString::number(ui->doubleSpinBox_spreadmin->value()));
        spread.append(QString::number(ui->doubleSpinBox_spreadmax->value()));
        spread.append(QString::number(ui->doubleSpinBox_a->value()));
    }

    QString random_wait = spread.join(",");
    int width = 450;
    int height = 350;
    int penwidth = 12;
    int columns = (int)((width - 2* penwidth)/penwidth);
    RandomNumber *rn = new RandomNumber();
    QList<double> dlist;
    QList<unsigned int> clist;
    for(int i = 0; i < columns; i++){
        clist.append(0);
    }
    double minx=ui->doubleSpinBox_spreadmin->value(), maxx=ui->doubleSpinBox_spreadmax->value();
    unsigned int maxy = 0;

    for(int i = 0; i < 10000; i++){
        double num = rn->random_wait_parser(random_wait);
        dlist.append(num);
        if(num > maxx)
            maxx=num;
        if(num < minx)
            minx=num;
    }

    double delta =(maxx-minx)/columns;
    foreach(double f , dlist){
        int current = floor( (f - minx) / delta );
        if(current == columns)
            current = columns - 1;
        clist.replace(current, clist.at(current) + 1);
        if(clist.at(current) > maxy)
            maxy = clist.at(current);
    }

    popup->resize(width,height);
    popup->show();


    l->setGeometry(0, 0, width, height);
    QPicture pi;
    QPainter p(&pi);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::black, penwidth, Qt::SolidLine, Qt::RoundCap));
    p.fillRect(0,0,width-penwidth,height-penwidth, Qt::white);

    for(int i = 0; i < columns; i++){
        p.drawPoint((i+1)*penwidth+1, height - floor(0.90*(clist.at(i)*(height-penwidth))/maxy));
    }
    p.end();

    l->setPicture(pi);
    l->show();

}

void AddThreadDialog::on_pushButton_clearsettings_clicked(){
    ui->lineEdit_title->setText("");
    ui->lineEdit_initialpage->setText("");
    ui->lineEdit_regex->setText("");

    ui->comboBox_spread->setCurrentIndex(0);
    ui->doubleSpinBox_spreadmin->setValue(5.0);
    ui->doubleSpinBox_spreadmax->setValue(60.0);

    ui->checkBox_safebrowsing->setChecked(false);
    ui->checkBox_saveallcontents->setChecked(false);
    ui->checkBox_savekeywords->setChecked(false);
    ui->checkBox_saveimgs->setChecked(false);
    ui->checkBox_robotcheck->setChecked(false);

    ui->plainTextEdit_header->setPlainText("");
}

void AddThreadDialog::on_pushButton_cancel_clicked(){
    hide();
    isATDactive = false;
    on_pushButton_clearsettings_clicked();
}

void AddThreadDialog::on_pushButton_add_clicked(){
    if(!ui->lineEdit_title->text().isEmpty() &&
            !ui->lineEdit_initialpage->text().isEmpty() &&
            !ui->lineEdit_regex->text().isEmpty() &&
            !ui->plainTextEdit_header->toPlainText().isEmpty()
            ){
        data.append(ui->lineEdit_title->text());
        data.append(ui->lineEdit_initialpage->text());
        data.append(ui->lineEdit_regex->text());

        QStringList spread;
        if(ui->comboBox_spread->currentText() == "Constant"){
            spread.append(ui->comboBox_spread->currentText());
            spread.append(QString::number(ui->doubleSpinBox_spreadmin->value()));
            spread.append(QString::number(ui->doubleSpinBox_spreadmax->value()));
        }else if(ui->comboBox_spread->currentText() == "Gauss"){
            spread.append(ui->comboBox_spread->currentText());
            spread.append(QString::number(ui->doubleSpinBox_spreadmin->value()));
            spread.append(QString::number(ui->doubleSpinBox_spreadmax->value()));
            spread.append(QString::number(ui->doubleSpinBox_mean->value()));
            spread.append(QString::number(ui->doubleSpinBox_sdev->value()));
        }else if(ui->comboBox_spread->currentText() == "Cauchy"){
            spread.append(ui->comboBox_spread->currentText());
            spread.append(QString::number(ui->doubleSpinBox_spreadmin->value()));
            spread.append(QString::number(ui->doubleSpinBox_spreadmax->value()));
            spread.append(QString::number(ui->doubleSpinBox_mean->value()));
            spread.append(QString::number(ui->doubleSpinBox_gamma->value()));
        }else if(ui->comboBox_spread->currentText() == "MaxBoltz"){
            spread.append(ui->comboBox_spread->currentText());
            spread.append(QString::number(ui->doubleSpinBox_spreadmin->value()));
            spread.append(QString::number(ui->doubleSpinBox_spreadmax->value()));
            spread.append(QString::number(ui->doubleSpinBox_a->value()));
        }
        data.append(spread.join(","));
        QStringList flags = QStringList();
        if(ui->checkBox_safebrowsing->isChecked())
            flags.append("safebrowsing");
        if(ui->checkBox_saveallcontents->isChecked())
            flags.append("saveallcontents");
        if(ui->checkBox_savekeywords->isChecked())
            flags.append("savekeywords");
        if(ui->checkBox_saveimgs->isChecked())
            flags.append("saveimgs");
        if(ui->checkBox_robotcheck->isChecked())
            flags.append("robotcheck");
        data.append(flags.join(","));
        data.append(ui->lineEdit_robotcheck->text());
        data.append(ui->plainTextEdit_header->toPlainText());
    }
    hide();
    on_pushButton_clearsettings_clicked();
    isATDactive = false;
}

