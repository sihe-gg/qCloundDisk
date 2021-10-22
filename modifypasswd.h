#ifndef MODIFYPASSWD_H
#define MODIFYPASSWD_H

#include <QWidget>
#include <QPainter>
#include <QIcon>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QRegExp>
#include <QDebug>

#include "logininfodata.h"

namespace Ui {
class ModifyPasswd;
}

class ModifyPasswd : public QWidget
{
    Q_OBJECT

public:
    explicit ModifyPasswd(QWidget *parent = nullptr);
    ~ModifyPasswd();

    int verifyData();
    void modifyNewPassword();

signals:
    void modifyPasswdSuccess();
protected:
    void paintEvent(QPaintEvent *);
private:
    Ui::ModifyPasswd *ui;

    // 单例模式
    LoginInfoData *loginInstance;
};

#endif // MODIFYPASSWD_H
