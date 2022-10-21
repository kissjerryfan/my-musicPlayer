
#ifndef LRC_WIDGET_H
#define LRC_WIDGET_H

#include <QWidget>

class lrc_widget : public QWidget
{
    Q_OBJECT
public:
    explicit lrc_widget(QWidget *parent = nullptr);
    void startLrc();
    void stopLrc();
    void setProportion(int x,int y);
    void setLabel_1_text(QString text);
    void setLabel_2_text(QString text);
private:
     QTimer* myTimer;
     double lrci;
     float Proportion;
     QString str1;
     QString str2;
     bool isFirst;
     bool isPress;
     QLinearGradient linearGradient;
     QLinearGradient maskLinearGradient;
     QFont font;
signals:

public slots:
     void doTimer();
};

#endif // LRC_WIDGET_H


