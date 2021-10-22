#ifndef LOGININFODATA_H
#define LOGININFODATA_H
#include <QString>
#include <QDebug>

class LoginInfoData
{
public:
    static LoginInfoData *getLoginInfoInstance();

    void setUserName(QString name);
    QString getUserName();
    void setPasswd(QString passwd);
    QString getPasswd();
    void setAddress(QString address);
    QString getAddress();

private:
    LoginInfoData();
    LoginInfoData(const LoginInfoData &LoginInfoData);

    static LoginInfoData *m_loginInfo;

    // 存储的数据
    QString m_user;
    QString m_address;
    QString m_passwd;


};

#endif // LOGININFODATA_H
