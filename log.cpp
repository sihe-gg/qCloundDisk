#include "log.h"
#include "ui_log.h"

log::log(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::log)
{
    ui->setupUi(this);

    initWindows();
    //设置样式表 CSS
    initStyleSheet();
    runWindows();
}

log::~log()
{
    delete ui;
}

void log::initWindows()
{
    ui->stackedWidget->setCurrentWidget(ui->page_Login);
    QApplication::setWindowIcon(QIcon(":/fileIcon/CloundDisk2.png"));
    m_config = new QSettings("UserConfig.ini", QSettings::IniFormat, this);
    //无边框
    setWindowFlags(Qt::FramelessWindowHint);
    //创建单例对象
    m_loginInstance = LoginInfoData::getLoginInfoInstance();

    connect(ui->closeBtn,&QPushButton::clicked,[=](){
        this->close();
    });
    connect(ui->minBtn,&QPushButton::clicked,[=](){
        this->showMinimized();
    });
    connect(ui->configBtn,&QPushButton::clicked,[=](){
        ui->loginUserName->clear();
        ui->loginPassword->clear();
        ui->stackedWidget->currentWidget() == ui->page_Login ? ui->stackedWidget->setCurrentWidget(ui->page_Server) : ui->stackedWidget->setCurrentWidget(ui->page_Login);
    });
    connect(ui->registerNowBtn,&QPushButton::clicked,[=](){
        ui->userName->clear();
        ui->nickName->clear();
        ui->password->clear();
        ui->confirmPwd->clear();
        ui->regMail->clear();
        ui->regPhone->clear();
        ui->stackedWidget->setCurrentWidget(ui->page_Registered);
    });
    connect(ui->serverAddr_PushButton, &QPushButton::clicked, [=](){
        QMessageBox::information(this, "服务器地址", "服务器地址设置成功，请登录！");
        m_loginInstance->setAddress(ui->address->text());
        ui->stackedWidget->setCurrentWidget(ui->page_Login);
    });
    connect(ui->autoGetAddr_PushButton, &QPushButton::clicked, this, &log::updateApplication);
    // ----------------------记住密码---------------------------
    // 检查配置记住密码文件ini是否存在
    QFileInfo iniFile("./UserConfig.ini");
    if(iniFile.exists())
    {
        initRememberPwd();
    }
}
void log::initStyleSheet()
{
    QString styleSheet = "log\
    {\
        border-image: url(:/images/login_bk.jpg);\
    }\
    log *\
    {\
        font-family: Arial;\
        font-size: 20px;\
    }\
    QLabel#label_2, #label_5, #label_11, #label_12\
    {\
        font-size: 30px;\
        font-weight: bold;\
    }\
    QLabel#imageLabel\
    {\
        border-image: url(:/images/13.png);\
    }\
    QLabel#label_2\
    {\
        color:white;\
    }\
    QLineEdit\
    {\
        font-size: 20px\
    }\
    QPushButton#registerNowBtn\
    {\
        color: orange;\
        text-decoration: underline;\
        border: 0px;\
    }\
    QPushButton#loginBtn, #registeredBtn, #serverConfigBtn\
    {\
        font-size: 25px;\
        border-image: url(:/images/balckButton.png);\
        width: 100px;\
        height: 40px;\
        color: white;\
    }\
    QCheckBox::indicator {\
          width: 60px;\
          height: 60px;\
    }\
    QCheckBox::indicator:unchecked\
    {\
          image: url(:/images/set2.png);\
    }\
    QCheckBox::indicator:checked\
    {\
          image: url(:/images/set3.png);\
    }";
    this->setStyleSheet(styleSheet);
}

int log::checkDatas(const int page)
{
    //取出用户输入的数据
    QString userName = ui->userName->text();
    QString nickName = ui->nickName->text();
    QString password = ui->password->text();
    QString confirmPwd = ui->confirmPwd->text();
    QString regMail = ui->regMail->text();
    QString regPhone = ui->regPhone->text();
    QString loginUserName = ui->loginUserName->text();
    QString loginPassword = ui->loginPassword->text();

    //数据校验
    QRegExp regExp;

    //判断登陆还是注册
    if(page == LOGIN_PAGE)
    {
        //校验用户名
        QString USER_REG = "^[a-zA-Z0-9_-]{4,16}$";
        regExp.setPattern(USER_REG);
        bool bl = regExp.exactMatch(loginUserName);
        if(bl == false)
        {
            QMessageBox::warning(this,"ERROR","用户名格式不正确！");
            return 1;
        }
        //校验密码
        QString PWD_REG = "^[a-zA-Z0-9_@-!#%^()=+\\*]{6,24}$";
        regExp.setPattern(PWD_REG);
        bl = regExp.exactMatch(loginPassword);
        if(bl == false)
        {
            QMessageBox::warning(this,"ERROR","用户名或者密码错误！");
            return 1;
        }
    }
    else
    {
        //校验用户名
        QString USER_REG = "^[a-zA-Z0-9_-]{4,16}$";
        regExp.setPattern(USER_REG);
        bool bl = regExp.exactMatch(userName);
        if(bl == false)
        {
            QMessageBox::warning(this,"ERROR","用户名格式不正确！");
            return 1;
        }
        //校验密码
        QString PWD_REG = "^[a-zA-Z0-9_@-!#%^()=+\\*]{6,24}$";
        regExp.setPattern(PWD_REG);
        bl = regExp.exactMatch(password);
        if(password != confirmPwd || bl == false)
        {
            QMessageBox::warning(this,"ERROR","密码格式不正确或者两次输入的密码不一致！");
            return 1;
        }
        //校验邮箱
        QString MAIL_REG = "^([A-Za-z0-9_-.])+@([A-Za-z0-9_-.])+.([A-Za-z]{2,4})$";
        regExp.setPattern(MAIL_REG);
        bl = regExp.exactMatch(regMail);
        if(bl == false)
        {
            QMessageBox::warning(this,"ERROR","邮箱格式不正确！");
            return 1;
        }
        //校验电话号码
        QString PHONE_REG = "^[0-9]{11}$";
        regExp.setPattern(PHONE_REG);
        bl = regExp.exactMatch(regPhone);
        if(bl == false)
        {
            QMessageBox::warning(this,"ERROR","手机号码不正确");
            return 1;
        }
    }

    return 0;
}

void log::sendReceiveDatas(const int page)
{
    //取出用户输入的数据
    QString userName = ui->userName->text();
    QString nickName = ui->nickName->text();
    QString password = ui->password->text();
    QString confirmPwd = ui->confirmPwd->text();
    QString regMail = ui->regMail->text();
    QString regPhone = ui->regPhone->text();
    QString loginUserName = ui->loginUserName->text();
    QString loginPassword = ui->loginPassword->text();
    QString date = QDateTime::currentDateTime().toString("yyyy-MM-dd&HH:mm:ss");

    QString url;
    //创建json对象
    QJsonObject obj;

    if(page == LOGIN_PAGE)
    {
        url = QString("http://%1/login").arg(ui->address->text());
        obj.insert("userName", loginUserName);
        obj.insert("password", loginPassword);
        obj.insert("loginDate", date);

        //单例模式赋值
        m_loginInstance->setAddress(ui->address->text());
        m_loginInstance->setPasswd(ui->loginPassword->text());
        m_loginInstance->setUserName(ui->loginUserName->text());
    }
    else
    {
        url = QString("http://%1/reg").arg(ui->address->text());
        obj.insert("userName", userName);
        obj.insert("nickName", nickName);
        obj.insert("password", password);
        obj.insert("regMail", regMail);
        obj.insert("regPhone", regPhone);
        obj.insert("registerDate", date);
    }

    //发送数据给服务器
    QNetworkAccessManager* pManager = new QNetworkAccessManager(this);
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    //obj->doc
    QJsonDocument doc(obj);
    //doc->QByteArray
    QByteArray json = doc.toJson();
    //发送数据给服务器
    QNetworkReply* reply = pManager->post(request,json);

    //取回服务器数据
    connect(reply,&QNetworkReply::readyRead,[=](){
        QByteArray all = reply->readAll();
        qDebug() << all;

        //QByteArray->doc
        QJsonDocument replyDoc = QJsonDocument::fromJson(all);
        //doc->json
        QJsonObject replyObj = replyDoc.object();
        QString status = replyObj.value("code").toString();

        qDebug()<<"status:"<<status;

        //判断成功，失败，给用户提醒
        if(status == "002")
        {
            QMessageBox::critical(this, "登录失败", "用户名或者密码错误，请重试！");
        }
        else if(status == "003")
        {
            if(ui->checkBox->isChecked())
            {
                saveRememberPwd();
            }
            m_uploadWindows = new upload;
            this->hide();
            m_uploadWindows->show();

            // 监听切换用户信号
            connect(m_uploadWindows, &upload::switchUser, [=](){
                this->show();
                m_uploadWindows->close();
                m_uploadWindows->deleteLater();
            });
        }
        else if(status == "004")
        {
            QMessageBox::critical(this, "注册失败", "用户名已经存在，请重试！");
        }
        else if(status == "005")
        {
            QMessageBox::critical(this, "注册失败", "邮箱已经被注册，请重试！");
        }
        else if(status == "006")
        {
            QMessageBox::critical(this, "注册失败", "手机号码已经被注册，请重试！");
        }
        else if(status == "007")
        {
            QMessageBox::information(this, "注册成功", "恭喜您注册成功，快去登录吧！！！");
        }
        else if(status == "008")
        {
            QMessageBox::critical(this, "服务器出现错误", "服务器可能正在维护，请稍后再试！");
        }
        else
        {
            QMessageBox::critical(this, "服务器出现错误", "服务器可能正在维护，请稍后再试！");
        }

        ui->loginBtn->setText("登录");
        ui->loginBtn->setEnabled(true);
        ui->registeredBtn->setText("注册");
        ui->registeredBtn->setEnabled(true);
        m_loginTimeOut = false;
    });

    // 服务器错误
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            [=](QNetworkReply::NetworkError code)
    {
        qDebug() << "code :" << code;
        if(code == QNetworkReply::HostNotFoundError)
        {
            QMessageBox::critical(this, "服务器出现错误", "无法找到服务器，服务器地址不正确，请重新输入！");
            ui->loginBtn->setText("登录");
            ui->loginBtn->setEnabled(true);
            ui->registeredBtn->setText("注册");
            ui->registeredBtn->setEnabled(true);

            m_loginTimeOut = false;
            reply->deleteLater();
            pManager->deleteLater();
            return;
        }
    });

    QTimer::singleShot(10000, this, [=](){
        if(m_loginTimeOut == false)
        {
            return;
        }
        QMessageBox::critical(this, "服务器出现错误", "服务器可能正在维护，请稍后再试！");
        ui->loginBtn->setText("登录");
        ui->loginBtn->setEnabled(true);
        ui->registeredBtn->setText("注册");
        ui->registeredBtn->setEnabled(true);

        reply->deleteLater();
        pManager->deleteLater();
        return;
    });
}

void log::runWindows()
{
    connect(ui->loginBtn,&QPushButton::clicked,[=](){
        ui->loginBtn->setText("登录中...");
        ui->loginBtn->setEnabled(false);
        //校验数据
        int flag = checkDatas(LOGIN_PAGE);
        if(flag == 1)        //校验失败
        {
            ui->loginBtn->setText("登录");
            ui->loginBtn->setEnabled(true);

            return;
        }

        sendReceiveDatas(LOGIN_PAGE);

        // 是否记住密码写入文件
        if(!ui->checkBox->isChecked())
        {
            cancelRememberPwd();
        }

    });
    connect(ui->registeredBtn,&QPushButton::clicked,[=](){
        ui->registeredBtn->setText("注册中...");
        ui->registeredBtn->setEnabled(false);
        //校验数据
        int flag = checkDatas(REGISTERED_PAGE);
        if(flag == 1)        //校验失败
        {
            ui->registeredBtn->setText("注册");
            ui->registeredBtn->setEnabled(true);

            return;
        }
        sendReceiveDatas(REGISTERED_PAGE);
    });
}

// 加密与解密
QString log::encryptAndUncrypt(QString src, const QChar key)
{
    for (int i = 0; i < src.count(); i++)
    {
        src[i] = src.at(i).toLatin1() ^ key.toLatin1();
    }

    return src;
}

// 初始化登录-记住密码
void log::initRememberPwd()
{
    QString initName = m_config->value("username").toString();
    QString initPwd = m_config->value("password").toString();
    QString remember = m_config->value("remember").toString();

    //解密
    initName = encryptAndUncrypt(initName, 'j');
    initPwd = encryptAndUncrypt(initPwd, 'j');

    if(remember == "true")
    {
        ui->checkBox->setChecked(true);
        ui->loginUserName->setText(initName);
        ui->loginPassword->setText(initPwd);
    }
}
void log::saveRememberPwd()
{
    QString initName = ui->loginUserName->text();
    QString initPwd = ui->loginPassword->text();
    QString remember = "true";

    //加密
    initName = encryptAndUncrypt(initName, 'j');
    initPwd = encryptAndUncrypt(initPwd, 'j');

    m_config->setValue("username", initName);
    m_config->setValue("password", initPwd);
    m_config->setValue("remember", remember);
    m_config->sync();
}
void log::cancelRememberPwd()
{
    m_config->setValue("remember", "false");
    m_config->sync();
}
// 自动获取服务器地址
void log::updateApplication()
{
    // 思路： 查询服务器文件，文件通过json格式发送,判断版本号是否更新
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QUrl url = QString("http://4o467u4639.zicp.fun/update");
    QNetworkRequest request(url);
    QNetworkReply *reply  = manager->get(request);
    connect(reply, &QNetworkReply::readyRead, [=](){
        QByteArray all = reply->readAll();
        // bytearray->doc
        QJsonDocument replyDoc = QJsonDocument::fromJson(all);
        // doc->obj
        QJsonObject replyObj = replyDoc.object();
        QString newAddr = replyObj.value("address").toString();
        ui->address->setText(newAddr);

        reply->deleteLater();
        manager->deleteLater();
    });

    // 下载错误检查
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            [=](QNetworkReply::NetworkError code)
    {
        qDebug() << "code :" << code;
        if(code == QNetworkReply::UnknownServerError || code == QNetworkReply::RemoteHostClosedError)
        {
            QMessageBox::critical(this, "自动获取", "服务器连接失败，请手动输入地址或者稍后再试！");
            reply->deleteLater();
            manager->deleteLater();
            return;
        }
    });
}

void log::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return)
    {
        ui->loginBtn->click();
    }
    else if(event->key() == Qt::Key_Enter)
    {
        ui->loginBtn->click();
    }
}

void log::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        m_point = e->globalPos() - this->frameGeometry().topLeft();
    }
}

void log::mouseMoveEvent(QMouseEvent *e)
{
    if(e->buttons() & Qt::LeftButton)
    {
        this->move(e->globalPos() - m_point);
    }
}
