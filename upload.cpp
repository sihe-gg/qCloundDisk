#include "upload.h"
#include "ui_upload.h"

int upload::m_fileNumber = 1;

upload::upload(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::upload)
{
    ui->setupUi(this);

    initUploadWindow();
    initUserFileList();
    addMenuAction();

    uploadFileData();
}

upload::~upload()
{
    delete ui;
}

void upload::initUploadWindow()
{
    // 设置css样式表
    QFile file("./upload.qss");
    file.open(QFile::ReadOnly);
    QString all = file.readAll();
    file.close();
    this->setStyleSheet(all);
    // 设置默认页
    ui->stackedWidget->setCurrentWidget(ui->myList_Page);
    // 设置鼠标跟踪
    this->setMouseTracking(true);
    // 获取登录信息
    loginInstance = LoginInfoData::getLoginInfoInstance();
    ui->uploadFontLabel->setText(loginInstance->getUserName());
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->resize(QSize(1100, 820));
    // 进度条初始化
    ui->progressBar->setValue(0);
    // QListWidget初始化
    ui->myFile_ListWidget->setProperty("contextMenuPolicy", Qt::CustomContextMenu);
    ui->myFile_ListWidget->setViewMode(QListView::IconMode);
    ui->myFile_ListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);//选择多个item
    ui->myFile_ListWidget->setSpacing(10);
    ui->myFile_ListWidget->setResizeMode(QListWidget::Adjust);
    ui->myFile_ListWidget->setIconSize(QSize(80, 80));

    ui->myShare_ListWidget->setProperty("contextMenuPolicy", Qt::CustomContextMenu);
    ui->myShare_ListWidget->setViewMode(QListView::IconMode);
    ui->myShare_ListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);//选择多个item
    ui->myShare_ListWidget->setSpacing(10);
    ui->myShare_ListWidget->setResizeMode(QListWidget::Adjust);
    ui->myShare_ListWidget->setIconSize(QSize(80, 80));
    ui->myShare_ListWidget->setDragEnabled(false);    //禁止拖拽

    ui->otherShare_ListWidget->setProperty("contextMenuPolicy", Qt::CustomContextMenu);
    ui->otherShare_ListWidget->setViewMode(QListView::IconMode);
    ui->otherShare_ListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);//选择多个item
    ui->otherShare_ListWidget->setSpacing(10);
    ui->otherShare_ListWidget->setResizeMode(QListWidget::Adjust);
    ui->otherShare_ListWidget->setIconSize(QSize(80, 80));
    ui->otherShare_ListWidget->setDragEnabled(false);    //禁止拖拽

    ui->uploadFile_ListWidget->setProperty("contextMenuPolicy", Qt::CustomContextMenu);
    ui->uploadFile_ListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);//选择多个item

    //=============================信号与槽===================================
    connect(ui->close_ToolButton, &QToolButton::clicked,[=](){
        this->close();
    });
    connect(ui->download_PushButton, &QPushButton::clicked,[=](){
        ui->stackedWidget->setCurrentWidget(ui->myList_Page);
    });
    connect(ui->upload_PushButton, &QPushButton::clicked,[=](){
        ui->stackedWidget->setCurrentWidget(ui->upload_page);
    });
    connect(ui->share_PushButton, &QPushButton::clicked,[=](){
        ui->stackedWidget->setCurrentWidget(ui->share_Page);
    });
    connect(ui->test_PushButton, &QPushButton::clicked,[=](){
        ui->stackedWidget->setCurrentWidget(ui->test_page);
    });

    // 获取上传文件路径
    connect(ui->getFilePath_PushButton, &QPushButton::clicked, this, &upload::getUploadFilePath);
    // 上传按钮点击
    connect(ui->uploadFile_PushButton, &QPushButton::clicked, this, &upload::uploadFileData);

//-------------------------------右键显示菜单项------------------------------

    connect(ui->myFile_ListWidget, &QListWidget::customContextMenuRequested, [=](){
        m_userMenu->exec(QCursor::pos());
    });
    connect(ui->uploadFile_ListWidget, &QListWidget::customContextMenuRequested, [=](){
        m_uploadMenu->exec(QCursor::pos());
    });
    connect(ui->myShare_ListWidget, &QListWidget::customContextMenuRequested, [=](){
        m_myShareMenu->exec(QCursor::pos());
    });
    connect(ui->otherShare_ListWidget, &QListWidget::customContextMenuRequested, [=](){
        m_otherShareMenu->exec(QCursor::pos());
    });
}

void upload::addMenuAction()
{
    // ---------图标设置栏-------------
    m_iconMenu = new QMenu(this);
    m_userInfoAction = new QAction("用户信息", this);
    m_updateAction = new QAction("检查更新", this);
    m_modifyPwdAction = new QAction("修改密码", this);
    m_switchUserAction = new QAction("切换登录", this);
    m_exitLoginAction = new QAction("退出", this);
    m_iconMenu->addAction(m_userInfoAction);
    m_iconMenu->addAction(m_updateAction);
    m_iconMenu->addAction(m_modifyPwdAction);
    m_iconMenu->addAction(m_switchUserAction);
    m_iconMenu->addAction(m_exitLoginAction);
    ui->icon_ToolButton->setMenu(m_iconMenu);
    connect(m_userInfoAction, &QAction::triggered, [=](){});
    connect(m_updateAction, &QAction::triggered, this, &upload::updateApplication);
    connect(m_modifyPwdAction, &QAction::triggered, [=](){
        m_modifyPwd = new ModifyPasswd;
        m_modifyPwd->show();

        // 监听修改密码信号
        connect(m_modifyPwd, &ModifyPasswd::modifyPasswdSuccess, [=](){
            m_modifyPwd->close();
            emit switchUser();
        });
    });
    connect(m_switchUserAction, &QAction::triggered, [=](){emit switchUser();});
    connect(m_exitLoginAction, &QAction::triggered, [=](){this->close();});
    // ---------用户区-------------
    m_userMenu = new QMenu(this);
    m_downloadAction = new QAction("下载", this);
    m_flushAction = new QAction("刷新", this);
    m_shareAction = new QAction("分享", this);
    m_delAction = new QAction("删除", this);
    m_propertyAction = new QAction("属性", this);
    m_userMenu->addAction(m_downloadAction);
    m_userMenu->addAction(m_flushAction);
    m_userMenu->addAction(m_shareAction);
    m_userMenu->addAction(m_delAction);
    m_userMenu->addAction(m_propertyAction);
    connect(m_downloadAction, &QAction::triggered, [=](){
        executeAction(m_downloadAction);
    });
    connect(m_shareAction, &QAction::triggered, [=](){
        executeAction(m_shareAction);
    });
    connect(m_delAction, &QAction::triggered, [=](){
        executeAction(m_delAction);
    });
    connect(m_propertyAction, &QAction::triggered, [=](){
        executeAction(m_propertyAction);
    });
    connect(m_flushAction, &QAction::triggered, [=](){
        m_fileNumber = 0;
        deleteList();
        initUserFileList();
    });
    // ---------分享区-------------
    m_myShareMenu = new QMenu(this);
    m_otherShareMenu = new QMenu(this);
    m_myShareCancelAction = new QAction("取消分享", this);
    m_myShareDownloadAction = new QAction("下载", this);
    m_otherShareDownloadAction = new QAction("下载", this);
    m_myShareMenu->addAction(m_myShareCancelAction);
    m_myShareMenu->addAction(m_myShareDownloadAction);
    m_myShareMenu->addAction(m_propertyAction);
    m_otherShareMenu->addAction(m_otherShareDownloadAction);
    m_otherShareMenu->addAction(m_propertyAction);
    connect(m_myShareCancelAction, &QAction::triggered, [=](){
        executeAction(m_myShareCancelAction);
    });
    connect(m_myShareDownloadAction, &QAction::triggered, [=](){
        executeAction(m_myShareDownloadAction);
    });
    connect(m_otherShareDownloadAction, &QAction::triggered, [=](){
        executeAction(m_otherShareDownloadAction);
    });
    // ---------上传区-------------
    m_uploadMenu = new QMenu(this);
    m_delFilePathAction = new QAction("删除", this);
    m_uploadMenu->addAction(m_delFilePathAction);
    connect(m_delFilePathAction, &QAction::triggered, [=](){
        QList<QListWidgetItem *> items = ui->uploadFile_ListWidget->selectedItems();
        QVector<QListWidgetItem *>::iterator ite = m_uploadVector.begin();

        if(items.count() > 0)
        {
            for (; ite != m_uploadVector.end(); ite++)
            {

                if(items.at(0) == *ite)
                {
                    qDebug() << "items.at(0)" << items.at(0) << "*ite" << *ite;
                    // 寻找vector中的QListWidgetItem位置
                    int removeNumberVector = m_uploadVector.indexOf(*ite);
                    // 从Vector中取出QListWidgetItem
                    QListWidgetItem *item = m_uploadVector.takeAt(removeNumberVector);
                    // 释放
                    delete item;

                    return;
                }
            }
        }
    });
}

void upload::executeAction(QAction *action)
{
    // 我的文件区创建selectedItems
    QList<QListWidgetItem *> myFileItems = ui->myFile_ListWidget->selectedItems();
    // 我的分享区创建selectedItems
    QList<QListWidgetItem *> myShareItems = ui->myShare_ListWidget->selectedItems();
    // 他人的分享区创建selectedItems
    QList<QListWidgetItem *> otherShareItems = ui->otherShare_ListWidget->selectedItems();

    qDebug() << "我的文件区被选中：" << myFileItems.size();

    if(myFileItems.size() > 0)
    {
        if(action == m_delAction)
        {
            m_message = new QMessageBox(this);
            m_message->setText("您正在删除文件");
            m_message->setInformativeText(QString("确定要删除这 %1 项吗?").arg(myFileItems.size()));
            m_message->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            m_message->setDefaultButton(QMessageBox::Yes);
            int ret = m_message->exec();
            switch (ret)
            {
            case QMessageBox::Yes:
                delFile(myFileItems);
                break;
            default:
                m_message->deleteLater();
                break;
            }
        }
        else if(action == m_downloadAction)
        {
            QString dir;
            m_message = new QMessageBox(this);
            m_message->setText("您正在下载文件");
            m_message->setInformativeText(QString("确定要下载这 %1 项吗?").arg(myFileItems.size()));
            m_message->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            m_message->setDefaultButton(QMessageBox::Yes);
            int ret = m_message->exec();

            switch (ret)
            {
            case QMessageBox::Yes:
                dir = QFileDialog::getExistingDirectory(this, "选择目录",
                        "C:/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
                downloadFile(myFileItems, dir, MAINFILE);
                break;
            default:
                m_message->deleteLater();
                break;
            }
        }
        else if(action == m_shareAction)
        {  
            m_message = new QMessageBox(this);
            m_message->setText("您正在分享文件");
            m_message->setInformativeText(QString("确定要分享这 %1 项吗?").arg(myFileItems.size()));
            m_message->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            m_message->setDefaultButton(QMessageBox::Yes);
            int ret = m_message->exec();
            switch (ret)
            {
            case QMessageBox::Yes:
                // 发送服务器用户分享的文件
                shareFile(myFileItems);
                break;
            default:
                m_message->deleteLater();
                break;
            }
        }
        else if(action == m_propertyAction)
        {
            fileAttribute(myFileItems, MAINFILE);
        }
    }

//----------------------------------------------------------------------------

    // 我的分享区有项目被选中
    if (myShareItems.size() > 0)
    {
        if(action == m_myShareCancelAction)
        {
            m_message = new QMessageBox(this);
            m_message->setText("您正在取消分享文件");
            m_message->setInformativeText(QString("确定要取消分享这 %1 项吗?").arg(myShareItems.size()));
            m_message->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            m_message->setDefaultButton(QMessageBox::Yes);
            int ret = m_message->exec();
            switch (ret)
            {
            case QMessageBox::Yes:
                cancelShareFile(myShareItems);
                break;
            default:
                m_message->deleteLater();
                break;
            }
        }
        else if(action == m_myShareDownloadAction)
        {
            QString dir;
            m_message = new QMessageBox(this);
            m_message->setText("您正在下载文件");
            m_message->setInformativeText(QString("确定要下载这 %1 项吗?").arg(myShareItems.size()));
            m_message->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            m_message->setDefaultButton(QMessageBox::Yes);
            int ret = m_message->exec();
            switch (ret)
            {
            case QMessageBox::Yes:
                dir = QFileDialog::getExistingDirectory(this, "选择目录",
                        "C:/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
                downloadFile(myShareItems, dir, SHAREFILE);
                break;
            default:
                m_message->deleteLater();
                break;
            }
        }
        else if(action == m_propertyAction)
        {
            fileAttribute(myShareItems, SHAREFILE);
        }
    }

//----------------------------------------------------------------------------

    // 他人的分享区有项目被选中
    if (otherShareItems.size() > 0)
    {
        if(action == m_otherShareDownloadAction)
        {
            QString dir;
            m_message = new QMessageBox(this);
            m_message->setText("您正在下载文件");
            m_message->setInformativeText(QString("确定要下载这 %1 项吗?").arg(otherShareItems.size()));
            m_message->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            m_message->setDefaultButton(QMessageBox::Yes);
            int ret = m_message->exec();
            switch (ret)
            {
            case QMessageBox::Yes:
                dir = QFileDialog::getExistingDirectory(this, "选择目录",
                        "C:/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
                downloadFile(otherShareItems, dir, SHAREFILE);
                break;
            default:
                m_message->deleteLater();
                break;
            }
        }
        else if(action == m_propertyAction)
        {
            fileAttribute(otherShareItems, SHAREFILE);
        }
    }
}

// 批量上传文件数据
void upload::uploadFileData()
{
    // 递归调用退出条件
    if(m_uploadVector.size() <= 0)
    {
        return;
    }
    // 取出一个项目准备上传
    QListWidgetItem *item = m_uploadVector.takeFirst();
    int currentItem = ui->uploadFile_ListWidget->row(item);

    // 准备数据
    QString filePath = ui->uploadFile_ListWidget->item(currentItem)->text();
    QFileInfo info(filePath);
    QString type = getSuffix(info.suffix());
    if(type == " ")
    {
        ui->receiveDataLine->append("文件格式无效，请重新上传!");
        return;
    }
    QString uploadDate = QDateTime::currentDateTime().toString("yyyy-MM-dd&HH:mm:ss");

    QString dispositionData = QString("form-data; name=%1; filename=%2; md5=%3; size=%4; date=%5")
            .arg(loginInstance->getUserName()).arg(info.fileName()).arg(getMd5(filePath))
            .arg(info.size()).arg(uploadDate);
    QUrl url((QString("http://%1/upload").arg(loginInstance->getAddress())));
    ui->receiveDataLine->append("开始上传，请稍后...");

    qDebug() << dispositionData;
    // 创建发送请求对象
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("multipart/form-data"));

    // multiPart类型http请求,传送文件
    QHttpMultiPart *multiPart = new QHttpMultiPart();
    QHttpPart part;

    part.setHeader(QNetworkRequest::ContentTypeHeader, type);
    part.setHeader(QNetworkRequest::ContentDispositionHeader, dispositionData);

    // 创建文件对象
    QFile *file = new QFile(filePath);
    bool ok = file->open(QIODevice::ReadOnly);
    if(!ok)
    {
        qDebug()<<"file open error";
        return;
    }
    part.setBodyDevice(file);
    file->setParent(multiPart);

    multiPart->append(part);

    // 创建访问管理对象
    QNetworkAccessManager *manage = new QNetworkAccessManager(this);
    // 管理者post服务端全部数据
    QNetworkReply *reply = manage->post(request, multiPart);
    multiPart->setParent(reply);

    //取回服务器数据
    connect(reply, &QNetworkReply::readyRead, [=](){
        //显示记录
        QByteArray all = reply->readAll();
        ui->receiveDataLine->append(all);
        ui->receiveDataLine->moveCursor(QTextCursor::Down);

        // 判断上传是否成功
        QString buf = all;
        //qDebug() << strncmp("成功！", buf.toUtf8().data(), strlen("成功！"));
        if(strncmp("成功！", buf.toUtf8().data(), strlen("成功！")) == 0)
        {
            // 将上传的文件添加到主界面
            addCommonFileList(info.fileName(), getMd5(filePath), 0, info.size(), uploadDate);
        }
    });

    //进度条处理
    connect(reply, &QNetworkReply::uploadProgress, [=](qint64 bytesSent, qint64 bytesTotal){
        if(bytesTotal == 0)
            return;

        ui->progressBar->setFormat(QStringLiteral("%1 : %p%").arg(filePath));
        ui->progressBar->setValue((bytesSent * 100 / bytesTotal));
    });
    // 上传文件完成
    connect(reply, &QNetworkReply::finished, [=](){
        ui->progressBar->setValue(0);

        // 释放QListWidgetItem
        delete item;

        // 清理内存，关闭文件
        file->close();
        reply->deleteLater();
        manage->deleteLater();
        multiPart->deleteLater();

        // 递归调用
        uploadFileData();
    });
}


// 获取用户文件列表
void upload::initUserFileList()
{
    // 客户端发送用户名字，查询用户上传过的文件
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QUrl url(QString("http://%1/userfile").arg(loginInstance->getAddress()));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));

    QByteArray userInfo(loginInstance->getUserName().toUtf8());

    QNetworkReply *reply = manager->post(request, userInfo);

    connect(reply, &QNetworkReply::readyRead, [=](){
        char buf[1024];
        qint64 line;

        while((line = reply->readLine(buf, 1024)) != 0)
        {
            ui->textEdit->append(buf);

            // 把文件显示到列表
            showFileList(buf);

            memset(buf, 0, 1024);
        }

        manager->deleteLater();
        reply->deleteLater();
    });
}

// 解析json，用户文件展示
void upload::showFileList(QByteArray data)
{
    /*     Json object          *
     *     count:20             *
     *                          *
     *     filename:132.txt     *
     *     username:qweqwe      *
     *     md5:xxx              *
     *     size:1000            *
     *                          *
     *     filename:abc.txt     *
     *     username:qweqwe      *
     *     md5:xxx              *
     *     size:200             *
     *                          *
     *     filename:hh.jpg      *
     *     username:qweqwe      *
     *     md5:xxx              *
     *     size:10              */

    // QByteArray -> doc
    QJsonDocument replyDoc = QJsonDocument::fromJson(data);
    // doc -> Json
    QJsonObject replyObj = replyDoc.object();

    // 解析最外层json
    // 解析发送的用户自己文件
    if(replyObj.contains("files"))
    {
        QJsonValue tmp = replyObj.value("files");
        QJsonObject obj = tmp.toObject();

        // 解析内层json
        QJsonValue filename = obj.value("filename");
        QJsonValue md5 = obj.value("md5");
        QJsonValue num = obj.value("num");
        QJsonValue fileSize = obj.value("size");
        QJsonValue uploadDate = obj.value("date");

        addCommonFileList(filename.toString(), md5.toString(),
                          num.toString().toInt(), fileSize.toString().toInt(), uploadDate.toString());
    }
    // 解析发送的分享文件
    else if(replyObj.contains("share"))
    {
        QJsonValue tmp = replyObj.value("share");
        QJsonObject obj = tmp.toObject();

        // 解析内层json
        QJsonValue shareUsername = obj.value("username");
        QJsonValue shareFilename = obj.value("sharefile");
        QJsonValue sharemd5 = obj.value("sharemd5");
        QJsonValue sharefileSize = obj.value("sharesize");
        QJsonValue downloadCount = obj.value("downloadcount");
        QJsonValue shareDate = obj.value("sharedate");

        addShareFileList(shareUsername.toString(), shareFilename.toString(), sharemd5.toString(),
                         sharefileSize.toString().toInt(), downloadCount.toString().toInt(), shareDate.toString());
    }

}

// 添加文件至主列表
void upload::addCommonFileList(QString filename, QString md5, int num, long fileSize, QString fileDate)
{
    QIcon icon;
    char *buf = filename.toUtf8().data();
    char *endBuf = buf + strlen(buf) - 4;
    QString suffix = endBuf;

    icon.addFile(getListIcon(endBuf));

    QListWidgetItem *item = new QListWidgetItem;

    item->setSizeHint(QSize(ICONWIDTH, ICONHEIGHT));
    item->setText(filename);
    item->setIcon(icon);

    // 创建自定义的类，存储服务器发回来的文件信息
    struct userFileInfo *fileInfo = new userFileInfo;

    fileInfo->m_username = loginInstance->getUserName();
    fileInfo->m_filename = filename;
    fileInfo->m_md5 = md5;
    fileInfo->m_suffix = suffix;
    fileInfo->m_date = fileDate;
    fileInfo->m_size = fileSize;
    fileInfo->m_list = item;
    fileInfo->m_isShare = num;

    m_vector.push_back(fileInfo);

    // 客户端显示文件列表
    ui->myFile_ListWidget->addItem(item);
}
// 添加共享文件到列表
void upload::addShareFileList(QString username, QString filename, QString md5,
                              long size, int downloadCount, QString shareDate)
{
    char *buf = filename.toUtf8().data();
    char *endBuf = buf + strlen(buf) - 4;
    QString suffix = endBuf;

    QIcon icon;
    icon.addFile(getListIcon(endBuf));

    // 创建列表item
    QListWidgetItem *item = new QListWidgetItem;

    item->setSizeHint(QSize(ICONWIDTH, ICONHEIGHT));
    item->setText(filename);
    item->setIcon(icon);

    // 创建userFileInfo
    struct userFileInfo *userShareInfo = new userFileInfo;

    userShareInfo->m_username = username;
    userShareInfo->m_filename = filename;
    userShareInfo->m_md5 = md5;
    userShareInfo->m_suffix = suffix;
    userShareInfo->m_date = shareDate;
    userShareInfo->m_size = size;
    userShareInfo->m_downloadCount = downloadCount;
    userShareInfo->m_list = item;

    qDebug() << "m_shareVector.size:" << m_shareVector.size();

    // 分享容器添加数据
    m_shareVector.push_back(userShareInfo);

    loginInstance->getUserName() == username ? ui->myShare_ListWidget->addItem(item) : ui->otherShare_ListWidget->addItem(item);
}


// 服务器端删除文件
void upload::delFile(QList<QListWidgetItem *> fileItems)
{
    // 递归调用退出条件
    if(fileItems.size() <= 0)
    {
        m_message->setText("删除文件完成，详情请点击查看更多！");
        m_message->setInformativeText("");
        m_message->setStandardButtons(QMessageBox::Ok);
        m_message->exec();

        // 释放
        m_message->deleteLater();
        return;
    }

    // 取出被选中的删除项
    QVector<userFileInfo *>::iterator delIte = m_vector.begin();
    QListWidgetItem *item = fileItems.takeFirst();

    for (; delIte != m_vector.end(); delIte++)
    {
        if(item == (*delIte)->m_list)
        {
            qDebug() << "fileItems: " << item << "downloadIte:" << (*delIte)->m_list;
            break;
        }

        // 如果到结尾没有此项，错误，退出
        if(delIte == m_vector.end())
        {
            m_message->setDetailedText(m_message->detailedText() + tr("删除错误，找不到指定文件，请重试！\n"));
            return;
        }
    }

    // 要删除的文件
    userFileInfo *delInfo = *delIte;

    // 创建访问管理对象
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    // 创建请求
    QNetworkRequest request;
    QUrl url(QString("http://%1/deletefile").arg(loginInstance->getAddress()));
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain"));

    // 删除发送格式:del username filename md5 date size /最后有空格
    QByteArray deleteFile = QString("%1 %2 %3 %4 %5 %6 ").arg("del").arg(loginInstance->getUserName()).
            arg(delInfo->m_filename).arg(delInfo->m_md5).arg(delInfo->m_date).
            arg(delInfo->m_size).toUtf8();

    //QByteArray deleteFile = QString(fileInfoList->m_filename + " " + loginInstance->getUserName()).toUtf8() + " ";
    qDebug() << QString(deleteFile);

    // 发送post
    QNetworkReply *reply = manager->post(request, deleteFile);
    connect(reply, &QNetworkReply::readyRead, [=](){
        QByteArray all = reply->readAll();
        ui->textEdit->append(all);
        // 获取detail，方便追加
        QString detail = m_message->detailedText();

        // QByteArray->doc
        QJsonDocument replyDoc = QJsonDocument::fromJson(all);
        // doc->json
        QJsonObject replyObj = replyDoc.object();

        QString status = replyObj.value("code").toString();
        if("001" == status)
        {
            ui->myFile_ListWidget->removeItemWidget(delInfo->m_list);
            // 从m_vector中移除
            int delNum = m_vector.indexOf(delInfo);
            m_vector.remove(delNum);
            delete delInfo->m_list;
            delete delInfo;

            m_message->setDetailedText(detail + tr("您选择的文件 %1 已经删除！\n").arg(delInfo->m_filename));
        }
        else if("002" == status)
        {
            m_message->setDetailedText(detail + tr("您选择的文件 %1 删除失败！\n").arg(delInfo->m_filename));
        }
        else
        {
            m_message->setDetailedText(detail + tr("服务器繁忙, 您选择的文件 %1 删除失败！\n").arg(delInfo->m_filename));
        }
    });

    connect(reply, &QNetworkReply::finished, [=](){
        reply->deleteLater();
        manager->deleteLater();

        delFile(fileItems);
    });

}

// 发送服务器要共享的文件
void upload::shareFile(QList<QListWidgetItem *> fileItems)
{
    // 递归调用退出条件
    if(fileItems.size() <= 0)
    {
        m_message->setText("分享完成，详情请点击查看更多！");
        m_message->setInformativeText("");
        m_message->setStandardButtons(QMessageBox::Ok);
        m_message->exec();

        // 释放
        m_message->deleteLater();
        return;
    }

    // 被选中的分享项
    QVector<userFileInfo *>::iterator shareIte = m_vector.begin();
    QListWidgetItem *takeItem = fileItems.takeFirst();

    for (; shareIte != m_vector.end(); shareIte++)
    {
        if(takeItem == (*shareIte)->m_list)
        {
            qDebug() << "fileItems: " << takeItem << "downloadIte:" << (*shareIte)->m_list;
            break;
        }

        // 如果到结尾没有此项，错误，退出
        if(shareIte == m_vector.end())
        {
            m_message->setDetailedText(m_message->detailedText() + tr("分享错误，找不到指定文件，请重试！\n"));
            return;
        }
    }
    // 要分享的文件
    userFileInfo *shareInfo = *shareIte;

    // 创建访问管理
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QUrl url(QString("http://%1/deletefile").arg(loginInstance->getAddress()));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

    /* 发送格式:share username filename md5 date size ,最后有一个空格*/
    QString shareDate = QDateTime::currentDateTime().toString("yyyy-MM-dd&HH:mm:ss");
    QByteArray data = QString("%1 %2 %3 %4 %5 %6 ").arg("share").arg(loginInstance->getUserName()).
            arg(shareInfo->m_filename).arg(shareInfo->m_md5).arg(shareDate).arg(shareInfo->m_size).toUtf8();

    qDebug() << "分享发送的数据是：" << data;

    QNetworkReply *reply = manager->post(request, data);

    connect(reply, &QNetworkReply::readyRead, [=](){
        // 获取之前细节方便追加
        QString detail = m_message->detailedText();

        QByteArray all = reply->readAll();
        ui->textEdit->append(all);

        QJsonDocument replyDoc = QJsonDocument::fromJson(all);
        QJsonObject replyObj = replyDoc.object();

        QString status = replyObj.value("code").toString();
        if("001" == status)
        {
            // 添加至共享列表
            QListWidgetItem *item = new QListWidgetItem;
            QIcon icon;
            icon.addFile(getListIcon(shareInfo->m_suffix.toUtf8().data()));
            item->setSizeHint(QSize(ICONWIDTH, ICONHEIGHT));
            item->setText(shareInfo->m_filename);
            item->setIcon(icon);

            // 创建userFileInfo
            struct userFileInfo *userShareInfo = new userFileInfo;
            userShareInfo->m_filename = shareInfo->m_filename;
            userShareInfo->m_username = loginInstance->getUserName();
            userShareInfo->m_md5 = shareInfo->m_md5;
            userShareInfo->m_suffix = shareInfo->m_suffix;
            userShareInfo->m_date = shareDate;
            userShareInfo->m_size = shareInfo->m_size;
            userShareInfo->m_downloadCount = 0;
            userShareInfo->m_isShare = true;
            userShareInfo->m_list = item;

            qDebug() << "m_shareVector.size:" << m_shareVector.size();
            // 分享容器添加数据
            m_shareVector.push_back(userShareInfo);
            // 分享区添加主题
            ui->myShare_ListWidget->addItem(item);

            m_message->setDetailedText(detail + tr("文件 %1 分享成功！\n").arg(shareInfo->m_filename));
        }
        else if("003" == status)
        {
            m_message->setDetailedText(detail + tr("文件 %1 已经被分享过了！\n").arg(shareInfo->m_filename));
        }
        else
        {
            m_message->setDetailedText(detail + tr("服务器出错，文件 %1 分享失败！\n").arg(shareInfo->m_filename));
        }
    });

    connect(reply, &QNetworkReply::finished, [=](){
        reply->deleteLater();
        manager->deleteLater();

        shareFile(fileItems);
    });

}

void upload::cancelShareFile(QList<QListWidgetItem *> fileItems)
{
    // 递归调用退出条件
    if(fileItems.size() <= 0)
    {
        m_message->setText("取消分享完成，详情请点击查看更多！");
        m_message->setInformativeText("");
        m_message->setStandardButtons(QMessageBox::Ok);
        m_message->exec();

        // 释放
        m_message->deleteLater();
        return;
    }

    // 取出被选中的取消分享项
    QVector<userFileInfo *>::iterator cancelShareIte = m_shareVector.begin();
    QListWidgetItem *item = fileItems.takeFirst();

    for (; cancelShareIte != m_shareVector.end(); cancelShareIte++)
    {
        if(item == (*cancelShareIte)->m_list)
        {
            qDebug() << "fileItems: " << item << "downloadIte:" << (*cancelShareIte)->m_list;
            break;
        }

        // 如果到结尾没有此项，错误，退出
        if(cancelShareIte == m_shareVector.end())
        {
            m_message->setDetailedText(m_message->detailedText() + tr("取消分享错误，找不到指定文件，请重试！\n"));
            return;
        }
    }
    // 取出要取消分享的文件
    userFileInfo *cancelShareInfo = *cancelShareIte;

    // 创建访问管理
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QUrl url(QString("http://%1/deletefile").arg(loginInstance->getAddress()));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

    /* 发送格式:cancelshare username filename md5 date size ,最后有一个空格*/
    QByteArray data = QString("%1 %2 %3 %4 %5 %6 ").arg("cancelshare").arg(loginInstance->getUserName()).
            arg(cancelShareInfo->m_filename).arg(cancelShareInfo->m_md5).
            arg(cancelShareInfo->m_date).arg(cancelShareInfo->m_size).toUtf8();

    qDebug() << "分享发送的数据是：" << data;

    QNetworkReply *reply = manager->post(request, data);

    connect(reply, &QNetworkReply::readyRead, [=](){
        QByteArray all = reply->readAll();
        ui->textEdit->append(all);

        // 获取之前细节方便追加
        QString detail = m_message->detailedText();

        QJsonDocument replyDoc = QJsonDocument::fromJson(all);
        QJsonObject replyObj = replyDoc.object();

        QString status = replyObj.value("code").toString();
        if("001" == status)
        {
            ui->myShare_ListWidget->removeItemWidget(cancelShareInfo->m_list);

            // 从vector中取出userFileInfo
            int cancelShareNumber = m_shareVector.indexOf(cancelShareInfo);
            m_shareVector.remove(cancelShareNumber);

            delete cancelShareInfo->m_list;
            delete cancelShareInfo;

            m_message->setDetailedText(detail + tr("取消分享文件 %1 成功！\n").arg(cancelShareInfo->m_filename));
        }
        else if("003" == status)
        {
            m_message->setDetailedText(detail + tr("取消分享文件 %1 失败！\n").arg(cancelShareInfo->m_filename));
        }
        else
        {
            m_message->setDetailedText(detail + tr("服务器繁忙，取消分享文件 %1 失败！\n").arg(cancelShareInfo->m_filename));
        }
    });

    connect(reply, &QNetworkReply::finished, [=](){
        reply->deleteLater();
        manager->deleteLater();

        cancelShareFile(fileItems);
    });

}

void upload::fileAttribute(QList<QListWidgetItem *> fileItems, int flag)
{
    if(fileItems.size() <= 0)
    {
        return;
    }

    QListWidgetItem *attributeItem = fileItems.takeFirst();
    QVector<userFileInfo *>::iterator ite;
    userFileInfo *attributeInfo;

    m_message = new QMessageBox(this);
    m_message->setText("文件属性");
    m_message->setStandardButtons(QMessageBox::Ok);
    m_message->setDefaultButton(QMessageBox::Ok);

    if(flag == MAINFILE)
    {
        for (ite = m_vector.begin(); ite != m_vector.end(); ite++)
        {
            if(attributeItem == (*ite)->m_list)
            {
                attributeInfo = *ite;
                m_message->setInformativeText(QString("文件名：%1\n"
                                                      "文件大小：%2\n"
                                                      "文件md5：%3\n"
                                                      "上传日期：%4\n"
                                                      "是否共享：%5").
                                              arg(attributeInfo->m_filename).arg(attributeInfo->m_size)
                                              .arg(attributeInfo->m_md5).arg(attributeInfo->m_date)
                                              .arg(attributeInfo->m_isShare == true ? "是":"否"));
                break;
            }
        }
    }
    else if(flag == SHAREFILE)
    {
        for (ite = m_shareVector.begin(); ite != m_vector.end(); ite++)
        {
            if(attributeItem == (*ite)->m_list)
            {
                attributeInfo = *ite;
                m_message->setInformativeText(QString("文件名：%1\n"
                                                      "分享来自于：%2\n"
                                                      "文件大小：%3\n"
                                                      "文件md5：%4\n"
                                                      "共享日期：%5\n"
                                                      "下载次数：%6").
                                              arg(attributeInfo->m_filename).arg(attributeInfo->m_username)
                                              .arg(attributeInfo->m_size).arg(attributeInfo->m_md5)
                                              .arg(attributeInfo->m_date).arg(attributeInfo->m_downloadCount));
                break;
            }
        }
    }
    else
    {
        return;
    }

    m_message->exec();
    m_message->deleteLater();

    fileAttribute(fileItems, flag);
}

void upload::updateApplication()
{
    // 思路： 查询服务器文件，文件通过json格式发送,判断版本号是否更新
    //QString oldVersion = "V0.1";
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QUrl url = QString("http://%1/update").arg(loginInstance->getAddress());
    QNetworkRequest request(url);
    QNetworkReply *reply  = manager->get(request);

    connect(reply, &QNetworkReply::readyRead, [=](){
        QByteArray all = reply->readAll();
        ui->textEdit->append(all);
        // bytearray->doc
        QJsonDocument replyDoc = QJsonDocument::fromJson(all);
        // doc->obj
        QJsonObject replyObj = replyDoc.object();
        QJsonValue value = replyObj.value("update");
        QJsonObject tempObj = value.toObject();
        QString newVersion = tempObj.value("version").toString();
        QString newUrl = tempObj.value("url").toString();
        QString newUpdateTime = tempObj.value("updatetime").toString();
        QString newNote = tempObj.value("releasenote").toString();

        qDebug() << newVersion << newUrl<<newUpdateTime<<newNote;
        if(VERSION < newVersion)
        {
            QString warningStr = "检测到新版本！\n版本号" + newVersion + "\n" + "更新时间：" + newUpdateTime
                    + "\n" + "更新说明：\n" + newNote;
            int ret = QMessageBox::warning(this, "检查更新", warningStr, "去下载", "不更新");

            if(ret == 0)//点击更新
            {
                qDebug() << "更新";
            }
            else
            {
                qDebug() << "不更新";
            }
        }
        else
        {
            QMessageBox::information(this, "检查更新", "当前已经是最新版本！");
        }

        reply->deleteLater();
        manager->deleteLater();
    });
}

// 获得文件路径
void upload::getUploadFilePath()
{
    QStringList filePathList = QFileDialog::getOpenFileNames(this, "请选择需要上传的文件", "C:/", "All files(*.*)");

    if(filePathList.size() <= 0)
    {
        qDebug() << "filePathList取消或者错误";
        return;
    }

    qDebug() << "stringList: " << filePathList;

    // 批量上传
    while(filePathList.size() > 0)
    {
        qDebug() << "filiePathSize:" << filePathList.size();
        QListWidgetItem *item = new QListWidgetItem(filePathList.takeFirst());
        ui->uploadFile_ListWidget->addItem(item);
        m_uploadVector.push_back(item);
    }
}

// 发送服务器要下载的文件
void upload::downloadFile(QList<QListWidgetItem *> fileItems, QString dir, int flag)
{
    if(dir == NULL)
    {
        qDebug() << "NULL";
        return;
    }
    // 递归调用退出条件
    if(fileItems.size() <= 0)
    {
        m_message->setText("下载文件完成，详情请点击查看更多！");
        m_message->setInformativeText("");
        m_message->setStandardButtons(QMessageBox::Ok);
        m_message->exec();

        // 释放
        m_message->deleteLater();
        return;
    }

    // 取出被选中的下载项
    QVector<userFileInfo *>::iterator downloadIte;
    QListWidgetItem *item = fileItems.takeFirst();;

    // 判断是主界面下载，还是分享下载
    if(flag == MAINFILE)
    {
        for (downloadIte = m_vector.begin(); downloadIte != m_vector.end(); downloadIte++)
        {
            if(item == (*downloadIte)->m_list)
            {
                qDebug() << "fileItems: " << item << "downloadIte:" << (*downloadIte)->m_list;
                break;
            }

            // 如果到结尾没有此项，错误，退出
            if(downloadIte == m_vector.end())
            {
                QMessageBox::warning(this, "下载", "下载错误，找不到指定文件，请重试！");
                return;
            }
        }
    }
    else if(flag == SHAREFILE)
    {
        for (downloadIte = m_shareVector.begin(); downloadIte != m_shareVector.end(); downloadIte++)
        {
            if(item == (*downloadIte)->m_list)
            {
                qDebug() << "fileItems: " << item << "downloadIte:" << (*downloadIte)->m_list;
                break;
            }

            // 如果到结尾没有此项，错误，退出
            if(downloadIte == m_shareVector.end())
            {
                QMessageBox::warning(this, "下载", "下载错误，找不到指定文件，请重试！");
                return;
            }
        }
    }
    else
    {
        qDebug() << "非法下载文件！";
        return;
    }

    // 取出要下载的文件
    userFileInfo *downloadInfo = *downloadIte;

    QString filePath = dir + "/" + downloadInfo->m_filename;
    QFile *file = new QFile(filePath);
    if(!file->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(this, "创建文件" , "创建文件失败，请重试！");

        file->deleteLater();
        return;
    }
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QUrl url(QString("http://%1/download").arg(loginInstance->getAddress()));
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

    // 下载格式:username filename md5 size 最后空格
    QByteArray downloadData = QString("%1 %2 %3 %4 ").arg(loginInstance->getUserName()).
            arg(downloadInfo->m_filename).arg(downloadInfo->m_md5).arg(downloadInfo->m_size).toUtf8();

    qDebug() << "download:" << downloadData;

    QNetworkReply *reply = manager->post(request, downloadData);

    connect(reply, &QNetworkReply::readyRead, [=](){
        qint64 size;
        while((size = reply->bytesAvailable()) > 0)
        {
            QByteArray all = reply->readAll();
            ui->textEdit->append(all);

            file->write(all);
        }
    });

    connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesReceived, qint64 bytesTotal){
        if(bytesTotal == 0)
            return;

        //bytesTotal == downloadInfo->m_size
        qint64 downloadPercentage = (bytesReceived * 100) / downloadInfo->m_size;
        ui->status_Label->setText(tr("文件: %1 正在下载: %2\%").arg(downloadInfo->m_filename).arg(QString::number(downloadPercentage)));
    });

    // 下载完成释放资源及递归调用下载
    connect(reply, &QNetworkReply::finished, [=](){
        m_message->setDetailedText(m_message->detailedText() + tr("文件 %1 下载成功！\n").arg(downloadInfo->m_filename));

        file->close();
        file->deleteLater();
        reply->deleteLater();
        manager->deleteLater();
        qDebug() << "下载完成，释放资源完成";

        // 下载完成递归调用下载
        downloadFile(fileItems, dir, flag);
    });

    // 下载错误检查
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            [=](QNetworkReply::NetworkError code)
    {
        qDebug() << "code :" << code;
        if(code == QNetworkReply::UnknownServerError)
        {
            m_message->setDetailedText(m_message->detailedText() + tr("连接服务器失败，文件 %1 下载失败！\n").arg(downloadInfo->m_filename));

            file->close();
            file->deleteLater();
            reply->deleteLater();
            manager->deleteLater();
            return;
        }
    });
}

QString upload::getListIcon(char *suffix)
{
    QString icon;

    // 添加图标
    if(strcmp(".JPG", suffix) == 0 || strcmp(".jpg", suffix) == 0)
        icon = QString(":/fileIcon/1image.png");
    else if(strcmp(".TXT", suffix) == 0 || strcmp(".txt", suffix) == 0)
        icon = QString(":/fileIcon/1Txt.png");
    else if(strcmp(".RAR", suffix) == 0 || strcmp(".rar", suffix) == 0 ||
            strcmp(".ZIP", suffix) == 0 || strcmp(".zip", suffix) == 0 ||
            strcmp(".7Z", suffix) == 0 || strcmp(".7z", suffix) == 0 ||
            strcmp(".TAR", suffix) == 0 || strcmp(".tar", suffix) == 0 ||
            strcmp(".GZ", suffix) == 0 || strcmp(".gz", suffix) == 0)
        icon = QString(":/fileIcon/1archive.png");
    else if(strcmp(".SQL", suffix) == 0 || strcmp(".sql", suffix) == 0 ||
            strcmp(".BAK", suffix) == 0 || strcmp(".bak", suffix) == 0 ||
            strcmp(".MDB", suffix) == 0 || strcmp(".mdb", suffix) == 0)
        icon = QString(":/fileIcon/1database.png");
    else if(strcmp(".MP3", suffix) == 0 || strcmp(".mp3", suffix) == 0)
        icon = QString(":/fileIcon/1audio.png");
    else if(strcmp(".XLSX", suffix) == 0 || strcmp(".xlsx", suffix) == 0)
        icon = QString(":/fileIcon/1Excel.png");
    else if(strcmp(".EXE", suffix) == 0 || strcmp(".exe", suffix) == 0)
        icon = QString(":/fileIcon/1Exe.png");
    else if(strcmp(".HTML", suffix) == 0 || strcmp(".html", suffix) == 0)
        icon = QString(":/fileIcon/1html.png");
    else if(strcmp(".ELF", suffix) == 0 || strcmp(".elf", suffix) == 0)
        icon = QString(":/fileIcon/1link.png");
    else if(strcmp(".PPT", suffix) == 0 || strcmp(".ppt", suffix) == 0 ||
            strcmp(".PPTX", suffix) == 0 || strcmp(".pptx", suffix) == 0)
        icon = QString(":/fileIcon/1PPT.png");
    else if(strcmp(".DOC", suffix) == 0 || strcmp(".doc", suffix) == 0 ||
            strcmp(".DOCX", suffix) == 0 || strcmp(".docx", suffix) == 0)
        icon = QString(":/fileIcon/1Word.png");
    else if(strcmp(".WPS", suffix) == 0 || strcmp(".wps", suffix) == 0 ||
            strcmp(".WPT", suffix) == 0 || strcmp(".wpt", suffix) == 0)
        icon = QString(":/fileIcon/1WPS.png");
    else if(strcmp(".MP4", suffix) == 0 || strcmp(".mp4", suffix) == 0 ||
            strcmp(".AVI", suffix) == 0 || strcmp(".avi", suffix) == 0 ||
            strcmp(".MKV", suffix) == 0 || strcmp(".mkv", suffix) == 0)
        icon = QString(":/fileIcon/1video.png");
    else
        icon = QString(":/fileIcon/unknowfile.png");

    return icon;
}

QString upload::getMd5(const QString path)
{
    // 获取文件md5
    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile file(path);
    file.open(QIODevice::ReadOnly);

    hash.addData(&file);

    QByteArray result = hash.result().toHex();

    return result;
}

QString upload::getSuffix(const QString suffix)
{
    QString type;

    if(suffix == "mp3")
        type = "audio/mp3";
    else if(suffix == "avi")
        type = "video/avi";
    else if(suffix == "html")
        type = "text/html";
    else if(suffix == "json")
        type = "application/json";
    else if(suffix == "exe")
        type = "application/x-msdownload";
    else if(suffix == "doc")
        type = "application/msword";
    else if(suffix == "gif" || suffix == "GIF")
        type = "image/gif";
    else if(suffix == "jpg" || suffix == "JPG")
        type = "application/x-jpg";
    else if(suffix == "mp4")
        type = "video/mpeg4";
    else if(suffix == "txt")
        type ="text/plain";
    else if(suffix == "apk")
        type ="application/vnd.android.package-archive";
    else if(suffix == "xls")
        type ="application/vnd.ms-excel";
    else if(suffix == "zip")
        type ="application/x-zip-compressed";
    else if(suffix == "xls")
        type ="application/x-xls";
    else
        type = " ";

    return type;
}

// 清空文件列表和item项目
void upload::deleteList()
{
    // 清空用户区
    int n = m_vector.size();
    for (int i = 0; i < n; i++)
    {
        userFileInfo *temp = m_vector.takeFirst();

        delete temp->m_list;
        delete temp;
    }
    // 清空分享区
    int shareNum = m_shareVector.size();
    for (int i = 0; i < shareNum; i++)
    {
        userFileInfo *temp = m_shareVector.takeFirst();

        delete temp->m_list;
        delete temp;
    }
}

// 临时打印容器信息
void upload::paintVector()
{
    QVector<userFileInfo *>::iterator ite = m_shareVector.begin();

    for (; ite != m_shareVector.end(); ite++)
    {
        qDebug() << "filename:" << (*ite)->m_filename << "QlistWidgetItem:" << (*ite)->m_list;
    }
}

void upload::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap pix(":/images/title_bk.jpg");
    painter.drawPixmap(0, 0, this->width(), this->height(), pix);
}

void upload::mouseMoveEvent(QMouseEvent *event)
{
    if(this->width() - 10 < event->pos().x() && event->pos().x() < this->width() &&
            this->height() - 10 < event->pos().y() && event->pos().y() < this->height())
    {
        this->setCursor(Qt::SizeFDiagCursor);
    }
    else
    {
        this->setCursor(Qt::ArrowCursor);
    }

    if(Qt::LeftButton)
    {
        if(m_bSizeOrMove)
        {
            if(m_bSize)
            {
                QPoint point = event->pos();
                if(point.x() > 200 && point.y() > 150)
                {
                    this->resize(point.x(),point.y());
                }
            }
        }
        else
        {
            if(m_bMove)
            {
                move(event->globalPos() - this->m_point);
            }
        }
    }
}

void upload::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        //拉伸
        if(this->width() - 10 < event->pos().x() && event->pos().x() < this->width() &&
                this->height() - 10 < event->pos().y() && event->pos().y() < this->height())
        {
            m_bSize = true;
            m_bSizeOrMove = true;
        }
        //移动
        else
        {
            m_bMove = true;
            m_bSizeOrMove = false;
            this->m_point = event->globalPos() - this->frameGeometry().topLeft();
        }
    }
}

void upload::mouseReleaseEvent(QMouseEvent *)
{
    m_bMove = false;
    m_bSize = false;
}
