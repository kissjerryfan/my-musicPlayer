#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include <QPainter>

namespace Ui {
class login;
}

class login : public QMainWindow
{
    Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    bool IsMatchRegX(const QString& str);
    ~login();
private slots:
    void paintEvent(QPaintEvent *event);
    void on_loginBtn_clicked();
    void slothide();
private:
    Ui::login *ui;
};

#endif // LOGIN_H
