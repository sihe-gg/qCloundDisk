#ifndef LOG_H
#define LOG_H

#include <QMainWindow>
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegExp>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QByteArray>
#include <QFile>
#include <QSettings>
#include <QTimer>

#include "upload.h"
#include "logininfodata.h"

const int LOGIN_PAGE = 1;
const int REGISTERED_PAGE = 2;



QT_BEGIN_NAMESPACE
namespace Ui { class log; }
QT_END_NAMESPACE

class log : public QMainWindow
{
    Q_OBJECT

public:
    log(QWidget *parent = nullptr);
    ~log();

    //初始化
    void initWindows();

    //校验注册内容
    int checkDatas(const int page);

    //服务器收发数据
    void sendReceiveDatas(const int page);

    //添加功能
    void runWindows();

    // base64异或加解密
    QString encryptAndUncrypt(QString src, const QChar key);

    // 记住密码
    void initRememberPwd();
    void saveRememberPwd();
    void cancelRememberPwd();

    // 升级
    void updateApplication();

protected:
    //重写鼠标按下事件
    void mousePressEvent(QMouseEvent *e);
    //重写鼠标移动事件
    void mouseMoveEvent(QMouseEvent *e);


private:
    Ui::log *ui;

    QPoint m_point;
    upload *m_uploadWindows;

    bool m_loginTimeOut = true;

    //创建单例对象
    LoginInfoData *m_loginInstance;
};
#endif // LOG_H
