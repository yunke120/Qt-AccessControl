#include "loginwidget.h"
#include "ui_loginwidget.h"

#include <QSettings>
#include <QDebug>
#include <QMessageBox>

LoginWidget::LoginWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginWidget)
{
    ui->setupUi(this);

    this->setWindowTitle(QStringLiteral("登录"));
    ui->lEPassword->setEchoMode(QLineEdit::Password);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::on_btnLogin_clicked()
{
    QSettings setting("Config.ini", QSettings::IniFormat);
    QString u = setting.value("USERNAME").toString();
    QString p = setting.value("PASSWORD").toString();

    QString u2 = ui->lEUserName->text();
    QString p2 = ui->lEPassword->text();
    if(u == u2 && p == p2)
    {
        qDebug() << "login success";
        w = new MainWindow;
        w->show();
        this->hide();
    }
    else {
        QMessageBox::warning(nullptr, QStringLiteral("登录"), QStringLiteral("登录失败"));
    }
}

void LoginWidget::on_btnConfig_clicked()
{
    w2 = new ConfigWidget;
    connect(w2, &ConfigWidget::sig_show, this, [=](){
        show();
    });
    w2->show();
    hide();
}
