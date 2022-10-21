
#include "lrc_widget.h"
#include<QTimer>
#include<QDebug>

lrc_widget::lrc_widget(QWidget *parent) : QWidget(parent)
{
    //歌词的线性渐变填充
    linearGradient.setStart(0,10);
    linearGradient.setFinalStop(0,40);
    linearGradient.setColorAt(0.1,QColor(14,179,255));
    linearGradient.setColorAt(0.5,QColor(14,32,255));
    linearGradient.setColorAt(0.9,QColor(14,179,255));

    //遮罩的线性渐变填充
    maskLinearGradient.setStart(0,10);
    maskLinearGradient.setFinalStop(0,40);
    maskLinearGradient.setColorAt(0.1,QColor(14,179,255));
    maskLinearGradient.setColorAt(0.5,QColor(14,32,255));
    maskLinearGradient.setColorAt(0.9,QColor(14,179,255));

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    font.setFamily("Times New Roman");
    font.setBold(true);
    font.setPointSize(30);

    isPress = false;
    str1 = QString("");
    str2 = QString("感谢聆听... ");

    myTimer=new QTimer(this);
    connect(myTimer,SIGNAL(timeout()),this,SLOT(doTimer()));
    lrci = 0;
}

void lrc_widget::startLrc()
{
    myTimer->start(1);
}
void lrc_widget::stopLrc()
{
    myTimer->stop();
}
void lrc_widget::doTimer()
{
    if(lrci < Proportion)
    {
        lrci+=1;
        update();
    }
    else
    {
        update();
    }
}

void lrc_widget::setProportion(int x, int y)
{
    Proportion = y;
    lrci = x;
    myTimer->start(1);
    qDebug()<<Proportion;
    update();
}

void lrc_widget::setLabel_1_text(QString text)
{
    str1 = text;
    isFirst = false;
    update();
}
void lrc_widget::setLabel_2_text(QString text)
{
    str2 = text;
    isFirst = true;
    update();
}
