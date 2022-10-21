
#include "login.h"
#include "ui_login.h"
#include "dialog.h"
#include <QPainter>
#include <QMessageBox>

login::login(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::login)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(),this->height());
    connect(ui->log_Btn,SIGNAL(clicked()),this,SLOT(on_loginBtn_clicked()));
}

void login::paintEvent(QPaintEvent *event)
{
    QPainter myPainter(this);
    myPainter.setOpacity(0.8);          //背景图片透明度
    myPainter.drawPixmap(0,0,this->width(),this->height(),QPixmap("/Qt1/MyPlayer01/images/bac01.jpg"));
}

void login::on_loginBtn_clicked()
{
    //如果用户名密码正确，则调用accept()槽函数。QDialog中的accept函数会关闭自身并发送QDialog::accepted标识
    //密码长度为8-16位，且必须为数字、大小写字母或符号中至少2种
    if (ui->line_name->text() == "username" && IsMatchRegX(ui->line_psw->text()))
       {
        QMessageBox::information(this,"欢迎","恭喜您登陆成功！");
        slothide();
        }
    else
    {
        QMessageBox::warning(this,"警告","用户名或密码错误！默认用户名为：username",QMessageBox::Yes);
        //清除内容并定位光标
        ui->line_name->clear();
        ui->line_psw->clear();
        ui->line_name->setFocus();
    }
}

void login::slothide()
{
    this->hide();
    Dialog* dia=new Dialog;
    dia->show();
}

bool login::IsMatchRegX(const QString& str)
{
    QString reg = "^(?![\\d]+$)(?![a-zA-Z]+$)(?![^\\da-zA-Z]+$).{8,16}$";
    QRegExp rx(reg);
    bool ok = rx.exactMatch(str);

    return ok;
}

login::~login()
{
    delete ui;
}

