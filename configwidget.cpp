#include "configwidget.h"
#include "ui_configwidget.h"

#include <QSettings>
#include <QMessageBox>

ConfigWidget::ConfigWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigWidget)
{
    ui->setupUi(this);
    this->setWindowTitle(QStringLiteral("修改密码"));
}

ConfigWidget::~ConfigWidget()
{
    delete ui;
}

void ConfigWidget::on_btnConfirm_clicked()
{
    QSettings setting("Config.ini", QSettings::IniFormat);
    QString u = setting.value("USERNAME").toString();

    QString u2 = ui->lEUsername->text();
    QString p2 = ui->lEPassword->text();

    if(u == u2 && p2 != "")
    {
        setting.setValue("PASSWORD", p2);
        QMessageBox::information(nullptr, QStringLiteral(""), QStringLiteral("修改成功，点击返回登录"));

    }
    else {
        QMessageBox::warning(nullptr, QStringLiteral(""), QStringLiteral("请检查输入用户名是否正确或密码不可为空"));
    }
}

void ConfigWidget::on_btnReturn_clicked()
{
    hide();
    emit sig_show();
}
