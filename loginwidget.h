#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>

#include "mainwindow.h"
#include "configwidget.h"
namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

private slots:
    void on_btnLogin_clicked();

    void on_btnConfig_clicked();

private:
    Ui::LoginWidget *ui;

    MainWindow *w;
    ConfigWidget *w2;
};

#endif // LOGINWIDGET_H
