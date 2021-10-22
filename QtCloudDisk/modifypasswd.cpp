#include "modifypasswd.h"
#include "ui_modifypasswd.h"

ModifyPasswd::ModifyPasswd(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ModifyPasswd)
{
    ui->setupUi(this);

    this->setFixedSize(500,200);
    this->setWindowTitle("修改密码");
    this->setWindowIcon(QIcon(":/images/icon.ico"));
    loginInstance = LoginInfoData::getLoginInfoInstance();
    this->setStyleSheet("ModifyPasswd *{font-family: Arial;font-size: 20px;color: black;font-weight:bold;}\
                        QLineEdit {\
                        border: 2px solid gray;\
                        border-radius: 10px;\
                        padding: 0 8px;\
                        background: #D1B8CB;\
                        color: #637DDB;\
                        selection-background-color: darkgray;}");


    connect(ui->confirm_PushButton, &QPushButton::clicked, this, &ModifyPasswd::modifyNewPassword);
    connect(ui->cancel_PushButton, &QPushButton::clicked, [=](){
        this->close();
    });

}

ModifyPasswd::~ModifyPasswd()
{
    delete ui;
}

// 判断密码是否符合条件
int ModifyPasswd::verifyData()
{
    QString oldPwd = ui->oldPwd_LineEdit->text();
    QString newPwd = ui->newPwd_LineEdit->text();
    QString repeat = ui->lineEdit_3->text();
    if(oldPwd != loginInstance->getPasswd())
    {
        QMessageBox::warning(this,"ERROR","原密码输入不正确！");
        return -1;
    }
    if(oldPwd == "" || newPwd == "" || repeat == "")
    {
        QMessageBox::warning(this,"ERROR","请填入有效的数据！");
        return -1;
    }
    if(oldPwd == newPwd)
    {
        QMessageBox::warning(this,"ERROR","新密码必须与旧密码不同！");
        return -1;
    }

    QRegExp regExp;

    QString PWD_REG = "^[a-zA-Z0-9_@-!#%^()=+\\*]{6,24}$";
    regExp.setPattern(PWD_REG);
    bool bl = regExp.exactMatch(newPwd);
    if(newPwd != repeat || bl == false)
    {
        QMessageBox::warning(this,"ERROR","密码格式不正确或者两次输入的密码不一致！");
        return -1;
    }

    return 0;
}

void ModifyPasswd::modifyNewPassword()
{
    int flag = -1;
    if((flag = verifyData()) != 0)
    {
        return;
    }

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QUrl url(QString("http://%1/modifypasswd").arg(loginInstance->getAddress()));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

    // 发送格式:username oldpassword newpassword ;
    QByteArray data = QString("%1 %2 %3 ").arg(loginInstance->getUserName())
            .arg(ui->oldPwd_LineEdit->text()).arg(ui->newPwd_LineEdit->text()).toUtf8();

    qDebug() << data;

    QNetworkReply *reply = manager->post(request, data);
    connect(reply, &QNetworkReply::readyRead, [=](){
        QByteArray replyData = reply->readAll();

        qDebug() << replyData;
        // QByteArray->doc
        QJsonDocument replyDoc = QJsonDocument::fromJson(replyData);
        // doc->obj
        QJsonObject obj = replyDoc.object();

        QString status = obj.value("code").toString();
        if(status == "001")
        {
            QMessageBox::information(this, "修改密码", "密码修改成功！");
            emit modifyPasswdSuccess();
        }
        else if(status == "002")
        {
            QMessageBox::information(this, "修改密码", "密码修改失败，请重试！");
        }
        else
        {
            QMessageBox::information(this, "修改密码", "服务器异常，请稍后再试！");
        }

        ui->oldPwd_LineEdit->clear();
        ui->newPwd_LineEdit->clear();
        ui->lineEdit_3->clear();
    });
}

void ModifyPasswd::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap pix(":/images/title_bk.jpg");

    painter.drawPixmap(0, 0, this->width(),this->height(), pix);
}
