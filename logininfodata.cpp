#include "logininfodata.h"

LoginInfoData *LoginInfoData::m_loginInfo = new LoginInfoData();

LoginInfoData *LoginInfoData::getLoginInfoInstance()
{
    return m_loginInfo;
}

void LoginInfoData::setUserName(QString name)
{
    this->m_user = name;
    qDebug()<<m_user;
}

QString LoginInfoData::getUserName()
{
    return m_user;
}

void LoginInfoData::setPasswd(QString passwd)
{
    this->m_passwd = passwd;
    qDebug()<<m_passwd;
}

QString LoginInfoData::getPasswd()
{
    return m_passwd;
}

void LoginInfoData::setAddress(QString address)
{
    this->m_address = address;
    qDebug() << address;
}

QString LoginInfoData::getAddress()
{
    return m_address;
}

LoginInfoData::LoginInfoData()
{

}
