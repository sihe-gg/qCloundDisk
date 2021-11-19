#include "upload.h"
#include "ui_upload.h"
#include <QMetaType>

int upload::m_fileNumber = 1;

upload::upload(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::upload)
{
    ui->setupUi(this);
    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<QList<QString>>("QList<QString>");

    initUploadWindow();
    initStyleSheet();    // 设置css样式表
    initThread();
    initUserFileList();

    readTransferConfigure();

    addMenuAction();
}

upload::~upload()
{
    m_transferThread->quit();
    m_transferThread->wait();

    delete ui;
}

void upload::initUploadWindow()
{
    // 设置默认页
    ui->stackedWidget->setCurrentWidget(ui->myList_Page);
    // 设置鼠标跟踪
    this->setMouseTracking(true);
    // 获取登录信息
    loginInstance = LoginInfoData::getLoginInfoInstance();
    ui->uploadFontLabel->setText(loginInstance->getUserName());
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->resize(QSize(1100, 820));
    // 初始化 config.ini
    m_config = new QSettings(QString("TransferList-%1.ini").arg(loginInstance->getUserName()), QSettings::IniFormat);
    m_config->setAtomicSyncRequired(true);
    // icon
    ui->uploadList_ToolButton->setText("上传列表");
    ui->uploadList_ToolButton->setIcon(QIcon(":/fileIcon/upload.png"));
    ui->downloadList_ToolButton->setText("下载列表");
    ui->downloadList_ToolButton->setIcon(QIcon(":/fileIcon/download.png"));
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

//-------------------------------- model-view-delegate -----------------------------------
    m_downloadTreeCurrentRow = 0;
    m_uploadTreeCurrentRow = 0;
    //-------------------------下载列表------------------------------------
    // model 初始化设置
    m_downloadModel = new QStandardItemModel();
    m_downloadModel->setColumnCount(COL_COUNT);
    m_downloadModel->setHeaderData(EMPTY_FIR, Qt::Horizontal, "#");
    m_downloadModel->setHeaderData(FILENAME, Qt::Horizontal, "名称");
    m_downloadModel->setHeaderData(FILESIZE, Qt::Horizontal, "文件大小");
    m_downloadModel->setHeaderData(FILESIZE, Qt::Horizontal, (int)(Qt::AlignRight|Qt::AlignVCenter), Qt::TextAlignmentRole);
    m_downloadModel->setHeaderData(PROGRESS, Qt::Horizontal, "进度");
    m_downloadModel->setHeaderData(OPERATE_FILE, Qt::Horizontal, "操作");
    // delegate 初始化
    m_downloadDelegate = new delegate(DOWNLOADLIST, ui->download_TreeView);
    // QTreeView 初始化
    ui->download_TreeView->setModel(m_downloadModel);
    ui->download_TreeView->setItemDelegate(m_downloadDelegate);
    ui->download_TreeView->setColumnWidth(EMPTY_FIR,50);
    ui->download_TreeView->setColumnWidth(FILENAME,300);
    ui->download_TreeView->setColumnWidth(FILESIZE,180);
    ui->download_TreeView->setColumnWidth(PROGRESS,150);
    ui->download_TreeView->setColumnWidth(OPERATE_FILE,120);
    ui->download_TreeView->setAlternatingRowColors(true);
    ui->download_TreeView->setMouseTracking(true);

    //-------------------------上传列表------------------------------------
    // model 初始化设置
    m_uploadModel = new QStandardItemModel();
    m_uploadModel->setColumnCount(COL_COUNT);
    m_uploadModel->setHeaderData(EMPTY_FIR, Qt::Horizontal, "#");
    m_uploadModel->setHeaderData(FILENAME, Qt::Horizontal, "名称");
    m_uploadModel->setHeaderData(FILESIZE, Qt::Horizontal, "文件大小");
    m_uploadModel->setHeaderData(FILESIZE, Qt::Horizontal, (int)(Qt::AlignRight|Qt::AlignVCenter), Qt::TextAlignmentRole);
    m_uploadModel->setHeaderData(PROGRESS, Qt::Horizontal, "进度");
    m_uploadModel->setHeaderData(OPERATE_FILE, Qt::Horizontal, "操作");
    // delegate 初始化
    m_uploadDelegate = new delegate(UPLOADLIST, ui->upload_TreeView);
    // QTreeView 初始化
    ui->upload_TreeView->setModel(m_uploadModel);
    ui->upload_TreeView->setItemDelegate(m_uploadDelegate);
    ui->upload_TreeView->setColumnWidth(EMPTY_FIR,30);
    ui->upload_TreeView->setColumnWidth(FILENAME,420);
    ui->upload_TreeView->setColumnWidth(FILESIZE,120);
    ui->upload_TreeView->setColumnWidth(PROGRESS,150);
    ui->upload_TreeView->setColumnWidth(OPERATE_FILE,100);
    ui->upload_TreeView->setAlternatingRowColors(true);
    ui->upload_TreeView->setMouseTracking(true);

    //=============================信号与槽===================================
    connect(ui->close_ToolButton, &QToolButton::clicked,[=](){
        this->close();
    });
    connect(ui->myFile_PushButton, &QPushButton::clicked,[=](){
        clearItemWidgets();
        ui->stackedWidget->setCurrentWidget(ui->myList_Page);
    });
    connect(ui->share_PushButton, &QPushButton::clicked,[=](){
        clearItemWidgets();
        ui->stackedWidget->setCurrentWidget(ui->share_Page);
    });
    connect(ui->test_PushButton, &QPushButton::clicked,[=](){
        clearItemWidgets();
        ui->stackedWidget->setCurrentWidget(ui->test_page);
    });
    connect(ui->transfer_PushButton, &QPushButton::clicked,[=](){
        clearItemWidgets();
        ui->stackedWidget->setCurrentWidget(ui->transfer_Page);
    });
    connect(ui->uploadList_ToolButton, &QToolButton::clicked, [=](){
        ui->stackedWidget_2->setCurrentWidget(ui->uploadList_Page);
    });
    connect(ui->downloadList_ToolButton, &QToolButton::clicked, [=](){
        ui->stackedWidget_2->setCurrentWidget(ui->downloadList_Page);
    });

//-------------------------------右键显示菜单项------------------------------
    connect(ui->myFile_ListWidget, &QListWidget::customContextMenuRequested, [=](const QPoint &pos){
        if(ui->myFile_ListWidget->itemAt(pos) != NULL)
            m_userMenu->exec(QCursor::pos());
        else
            m_blankMenu->exec(QCursor::pos());
    });
    connect(ui->myShare_ListWidget, &QListWidget::customContextMenuRequested, [=](const QPoint &pos){
        if(ui->myShare_ListWidget->itemAt(pos) != NULL)
            m_myShareMenu->exec(QCursor::pos());
    });
    connect(ui->otherShare_ListWidget, &QListWidget::customContextMenuRequested, [=](const QPoint &pos){
        if(ui->otherShare_ListWidget->itemAt(pos) != NULL)
            m_otherShareMenu->exec(QCursor::pos());
    });

//-------------------------------监听 delegate 下载列表-----------------------------
    connect(m_downloadDelegate, &delegate::open, [=](QModelIndex index){
        QVector<TransferTreeViewItem *>::iterator ite = m_downloadTreeVector.begin();
        for (; ite != m_downloadTreeVector.end(); ite++)
        {
            if((*ite)->m_row == index.row())
            {
                break;
            }
        }
        QString filePath = (*ite)->m_dirUrl + "/" + (*ite)->m_treeFileName;
        QFile checkFile(filePath);
        if(checkFile.exists())
        {
            /* 斜杠转换 /->\\      */
            filePath = QDir::toNativeSeparators(filePath);
            QProcess process;
            process.startDetached("explorer.exe", QStringList() << "/select," << filePath);
        }
        else
        {
            QMessageBox::information(this, "打开", tr("打开文件 '%1' 失败,文件不存在!").arg((*ite)->m_treeFileName));
        }
    });
    connect(m_downloadDelegate, &delegate::delData, [=](QModelIndex index){
        QVector<TransferTreeViewItem *>::iterator ite = m_downloadTreeVector.begin();
        for (; ite != m_downloadTreeVector.end(); ite++)
        {
            if((*ite)->m_row == index.row())
            {
                break;
            }
        }
        int ret = QMessageBox::information(this, "删除", tr("您将从下载列表删除 '%1' ,是否同时也在硬盘上删除文件?").arg((*ite)->m_treeFileName),
                                 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No);

        QList<QStandardItem *> takeItems;
        QString dir = (*ite)->m_dirUrl + "/" + (*ite)->m_treeFileName;
        QFile file(dir);

        // 获取在 DownloadTreeViewItem 上的行数
        int currentRow = m_downloadTreeVector.indexOf(*ite);
        struct TransferTreeViewItem *dTreeView;

        switch(ret)
        {
        case QMessageBox::Yes:
            // 从 DownloadTreeViewItem 取走 item
            takeItems = m_downloadModel->takeRow(currentRow);
            for (int i = 0; i < takeItems.size(); i++)
            {
                delete takeItems[i];
            }

            // 从 m_downloadTreeVector 中取走
            dTreeView = m_downloadTreeVector.takeAt(currentRow);
            delete dTreeView;
            // 删除硬盘文件
            if(file.exists())
            {
               file.remove();
            }
            // 更新
            updateCurrentRow(DOWNLOADLIST);
            updateConfigFile(DOWNLOADLIST);
            m_downloadTreeCurrentRow--;

            break;
        case QMessageBox::No:
            takeItems = m_downloadModel->takeRow(currentRow);
            for (int i = 0; i < takeItems.size(); i++)
            {
                delete takeItems[i];
            }
            // 从 m_downloadTreeVector 中取走
            dTreeView = m_downloadTreeVector.takeAt(currentRow);
            delete dTreeView;
            // 更新
            updateCurrentRow(DOWNLOADLIST);
            updateConfigFile(DOWNLOADLIST);
            m_downloadTreeCurrentRow--;

            break;
        case QMessageBox::Cancel:
            break;
        default:
            break;
        }
    });
    // 下载列表双击打开
    connect(ui->download_TreeView, &QAbstractItemView::doubleClicked, [=](const QModelIndex index){
        QVector<TransferTreeViewItem *>::iterator ite = m_downloadTreeVector.begin();
        for (; ite != m_downloadTreeVector.end(); ite++)
        {
            if((*ite)->m_row == index.row())
            {
                break;
            }
        }
        QString filePath = (*ite)->m_dirUrl + "/" + (*ite)->m_treeFileName;
        QFile checkFile(filePath);
        if(checkFile.exists())
        {
            /* 斜杠转换 /->\\      */
            filePath = QDir::toNativeSeparators(filePath);
            QProcess process;
            process.startDetached("explorer.exe", QStringList() << filePath);
        }
        else
        {
            QMessageBox::information(this, "打开", tr("打开文件 '%1' 失败,文件不存在!").arg((*ite)->m_treeFileName));
        }
    });
//---------------------------------------监听 delegate 上传列表----------------------------------
    connect(m_uploadDelegate, &delegate::open, [=](QModelIndex index){
        QVector<TransferTreeViewItem *>::iterator ite = m_uploadTreeVector.begin();
        for (; ite != m_uploadTreeVector.end(); ite++)
        {
            if((*ite)->m_row == index.row())
            {
                break;
            }
        }
        for (QVector<UserFileInfo *>::iterator mainIte = m_vector.begin(); mainIte != m_vector.end(); mainIte++)
        {
            // 对比上传列表区和文件区 md5 匹配删除
            if((*ite)->m_treeMd5 == (*mainIte)->m_md5)
            {
                fileAttribute(*mainIte, MAINFILE);
                break;
            }
        }
    });
    connect(m_uploadDelegate, &delegate::delData, [=](QModelIndex index){
        QVector<TransferTreeViewItem *>::iterator ite = m_uploadTreeVector.begin();
        for (; ite != m_uploadTreeVector.end(); ite++)
        {
            if((*ite)->m_row == index.row())
            {
                break;
            }
        }
        int ret = QMessageBox::information(this, "删除", tr("是否删除上传列表文件 '%1' ?").arg((*ite)->m_dirUrl),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        QList<QStandardItem *> takeItems;
        int currentRow = m_uploadTreeVector.indexOf(*ite);
        struct TransferTreeViewItem *uTreeView;

        switch (ret)
        {
        case QMessageBox::Yes:
            for (QVector<UserFileInfo *>::iterator delIte = m_vector.begin(); delIte != m_vector.end(); delIte++)
            {
                // 对比上传列表区和文件区 md5 匹配删除
                if((*ite)->m_treeMd5 == (*delIte)->m_md5)
                {
                    m_messageCount = 1;
                    m_message = new QMessageBox(this);
                    m_message->setStandardButtons(QMessageBox::Ok);
                    delFile(*delIte);
                    break;
                }
            }
            // 列表上取出
            takeItems = m_uploadModel->takeRow(currentRow);
            for (int i = 0; i < takeItems.size(); i++)
            {
                delete takeItems[i];
            }
            // Vector中取出
            uTreeView = m_uploadTreeVector.takeAt(currentRow);
            delete uTreeView;

            updateCurrentRow(UPLOADLIST);
            updateConfigFile(UPLOADLIST);
            m_uploadTreeCurrentRow--;
            break;
        case QMessageBox::No:
            break;
        default:
            break;
        }

    });
}

// 初始化线程
void upload::initThread()
{
    m_transferThread = new QThread();
    m_worker = new MultiThread();

    m_worker->moveToThread(m_transferThread);

    connect(this, &upload::startRunning, m_worker, &MultiThread::startDownload);
    connect(this, &upload::uploadStartRunning, m_worker, &MultiThread::startUpload);
    connect(m_transferThread, &QThread::finished, m_worker, &QObject::deleteLater);

    m_transferThread->start();
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

    // ---------空白区-------------
    m_blankMenu = new QMenu(this);
    m_flushAction = new QAction("刷新页面", this);
    m_uploadFileAction = new QAction("上传文件", this);
    m_blankMenu->addAction(m_uploadFileAction);
    m_blankMenu->addAction(m_flushAction);
    connect(m_flushAction, &QAction::triggered, [=](){
        m_fileNumber = 0;
        deleteList();
        initUserFileList();
    });
    connect(m_uploadFileAction, &QAction::triggered, this, &upload::uploadFileData);

    // ---------用户区-------------
    m_userMenu = new QMenu(this);
    m_downloadAction = new QAction("下载", this);
    m_shareAction = new QAction("分享", this);
    m_delAction = new QAction("删除", this);
    m_propertyAction = new QAction("属性", this);
    m_userMenu->addAction(m_downloadAction);
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
    // ---------分享区-------------
    m_myShareMenu = new QMenu(this);
    m_otherShareMenu = new QMenu(this);
    m_myShareDownloadAction = new QAction("下载文件", this);
    m_myShareCancelAction = new QAction("取消分享", this);
    m_otherShareDownloadAction = new QAction("下载文件", this);
    m_myShareMenu->addAction(m_myShareDownloadAction);
    m_myShareMenu->addAction(m_myShareCancelAction);
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
}

// 读取下载区列表配置
void upload::readTransferConfigure()
{
    QFile configFile(QString("./TransferList-%1.ini").arg(loginInstance->getUserName()));
    if(!configFile.exists())
    {
        return;
    }
    // 下载列表
    int downloadListSize = m_config->beginReadArray("downloadTreeList");
    for (int i = 0; i < downloadListSize; i++)
    {
        m_config->setArrayIndex(i);

        QString dirUrl = m_config->value("dirUrl").toString();
        QString filename = m_config->value("fileName").toString();
        //int row = m_config->value("row").toInt();
        int value = 100;
        qint64 size = m_config->value("fileSize").toULongLong();

        checkDownloadViewItem(filename, size, dirUrl, value);
    }
    m_config->endArray();

    // 上传列表
    int uploadListSize = m_config->beginReadArray("uploadTreeList");
    for (int i = 0; i < uploadListSize; i++)
    {
        m_config->setArrayIndex(i);

        QString dirUrl = m_config->value("dirUrl").toString();
        //QString filename = m_config->value("fileName").toString();
        //int row = m_config->value("row").toInt();
        int value = 100;
        qint64 size = m_config->value("fileSize").toULongLong();
        QString md5 = m_config->value("md5").toString();

        checkUploadViewItem(dirUrl, md5, size, value);
    }
    m_config->endArray();
}

void upload::executeAction(QAction *action)
{
    // 我的文件区创建 selectedItems
    QList<QListWidgetItem *> myFileItems = ui->myFile_ListWidget->selectedItems();
    QVector<UserFileInfo *>::iterator ite;
    QString dir;
    // 我的分享区创建 selectedItems
    QList<QListWidgetItem *> myShareItems = ui->myShare_ListWidget->selectedItems();
    QVector<UserFileInfo *>::iterator shareIte;
    // 他人的分享区创建 selectedItems
    QList<QListWidgetItem *> otherShareItems = ui->otherShare_ListWidget->selectedItems();

    // 创建 QMessagebox 记录 share、cancelShare、del 结果
    if(action == m_delAction || action == m_shareAction || action == m_myShareCancelAction)
    {
        m_message = new QMessageBox(this);
        m_message->setStandardButtons(QMessageBox::Ok);
    }

    QList<QListWidgetItem *> tempItems = myFileItems.size() > 0 ? myFileItems:myShareItems.size() > 0 ? myShareItems:otherShareItems;
    m_messageCount = tempItems.size();
    qDebug() << "m_messageCount"<<m_messageCount;

    // 选择下载目录
    if(action == m_downloadAction || action == m_myShareDownloadAction || action == m_otherShareDownloadAction)
    {
        int ret = QMessageBox::information(this, "下载", tr("您确定下载这 %1 项吗？").
                                           arg(QString::number(tempItems.size())),
                                           QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        switch (ret)
        {
        case QMessageBox::No:
            for (int i = 0; i < tempItems.size(); i++)
            {
                tempItems[i]->setSelected(false);
            }
            return;
        default:
            break;
        }
        dir = QFileDialog::getExistingDirectory(this, "选择目录",
                                                QDir::currentPath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    }

//----------------------------------------------------------------------------

    // 我的文件区有项目被选中
    while(myFileItems.size() > 0)
    {
        QListWidgetItem *item = myFileItems.takeFirst();

        for (ite = m_vector.begin(); ite != m_vector.end() ; ite++)
        {
            if(item == (*ite)->m_list)
            {
                break;          //找到一个QListWidgetItem
            }
        }

        if(action == m_delAction)
        {
            delFile(*ite);
        }
        else if(action == m_downloadAction)
        {
            downloadFile(*ite, dir);
        }
        else if(action == m_shareAction)
        {
            shareFile(*ite);
        }
        else if(action == m_propertyAction)
        {
            fileAttribute(*ite, MAINFILE);
        }

        // 取消被选中状态(多个 QListWidget 切换时可能发生多个选中状态)
        item->setSelected(false);
    }

//----------------------------------------------------------------------------

    // 我的分享区有项目被选中
    while(myShareItems.size() > 0)
    {
        QListWidgetItem *item = myShareItems.takeFirst();

        for (shareIte = m_shareVector.begin(); shareIte != m_shareVector.end(); shareIte++)
        {
            if(item == (*shareIte)->m_list)
            {
                break;              //找到项目
            }
        }

        if(action == m_myShareCancelAction)
        {
            cancelShareFile(*shareIte);
        }
        else if(action == m_myShareDownloadAction)
        {
            downloadFile(*shareIte, dir);
        }
        else if(action == m_propertyAction)
        {
            fileAttribute(*shareIte, SHAREFILE);
        }

        // 取消被选中状态(多个 QListWidget 切换时可能发生多个选中状态)
        item->setSelected(false);
    }

//----------------------------------------------------------------------------

    // 他人的分享区有项目被选中
    while(otherShareItems.size() > 0)
    {
        QListWidgetItem *item = otherShareItems.takeFirst();

        for (shareIte = m_shareVector.begin(); shareIte != m_shareVector.end(); shareIte++)
        {
            if(item == (*shareIte)->m_list)
            {
                break;              //找到项目
            }
        }

        if(action == m_otherShareDownloadAction)
        {
            downloadFile(*shareIte, dir);
        }
        else if(action == m_propertyAction)
        {
            fileAttribute(*shareIte, SHAREFILE);
        }

        // 取消被选中状态(多个 QListWidget 切换时可能发生多个选中状态)
        item->setSelected(false);
    }
}

// 批量上传文件数据
void upload::uploadFileData()
{
    QStringList filePathList = QFileDialog::getOpenFileNames(this, "请选择需要上传的文件", "C:/", "All files(*.*)");

    if(filePathList.size() <= 0)
    {
        return;
    }

    emit uploadStartRunning(filePathList, loginInstance->getUserName(), loginInstance->getAddress());

    // 添加上传列表 item
    connect(m_worker, &MultiThread::uploadFileInfo, this, &upload::addUploadFileInfo, Qt::UniqueConnection);

    // 将上传的文件添加到主界面
    connect(m_worker, &MultiThread::uploadResultReady, this, &upload::addCommonFileList, Qt::UniqueConnection);
    // set progressBar
    connect(m_worker, &MultiThread::sendUploadProgress, this, &upload::uploadSetProgressBar, Qt::UniqueConnection);
    // clear
    connect(m_worker, &MultiThread::uploadFinish, this, &upload::uploadListFinish, Qt::UniqueConnection);
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
void upload::addCommonFileList(QString filename, QString md5, int num, qint64 fileSize, QString fileDate)
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
    struct UserFileInfo *fileInfo = new UserFileInfo;

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
                              qint64 size, int downloadCount, QString shareDate)
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

    // 创建UserFileInfo
    struct UserFileInfo *userShareInfo = new UserFileInfo;

    userShareInfo->m_username = username;
    userShareInfo->m_filename = filename;
    userShareInfo->m_md5 = md5;
    userShareInfo->m_suffix = suffix;
    userShareInfo->m_date = shareDate;
    userShareInfo->m_size = size;
    userShareInfo->m_downloadCount = downloadCount;
    userShareInfo->m_list = item;

    // 分享容器添加数据
    m_shareVector.push_back(userShareInfo);

    loginInstance->getUserName() == username ? ui->myShare_ListWidget->addItem(item) : ui->otherShare_ListWidget->addItem(item);
}

void upload::checkUploadViewItem(const QString filePath, const QString md5, const qint64 fileSize, const int value)
{
    if(m_uploadTreeVector.size() > 0)
    {
        QVector<TransferTreeViewItem *>::iterator ite = m_uploadTreeVector.begin();
        for (; ite != m_uploadTreeVector.end(); ite++)
        {
            // 有重复项目
            if((*ite)->m_treeMd5 == md5)
            {
                (*ite)->m_value = 0;
            }
        }
    }
    QStandardItem *treeEmpty_First = new QStandardItem("*");
    treeEmpty_First->setEditable(false);
    QStandardItem *treeFileName = new QStandardItem(filePath);
    treeFileName->setEditable(false);
    QStandardItem *treeFileSize = new QStandardItem(humanFileSize(fileSize));
    treeFileSize->setEditable(false);
    treeFileSize->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
    QStandardItem *treeProgressBar = new QStandardItem("0");
    treeProgressBar->setEditable(false);
    QStandardItem *treeOperate_File = new QStandardItem("");
    treeOperate_File->setEditable(false);

    m_uploadModel->setItem(m_uploadTreeCurrentRow, EMPTY_FIR, treeEmpty_First);
    m_uploadModel->setItem(m_uploadTreeCurrentRow, FILENAME, treeFileName);
    m_uploadModel->setItem(m_uploadTreeCurrentRow, FILESIZE, treeFileSize);
    m_uploadModel->setItem(m_uploadTreeCurrentRow, PROGRESS, treeProgressBar);
    m_uploadModel->setData(m_uploadModel->index(m_uploadTreeCurrentRow, PROGRESS), value);
    m_uploadModel->setItem(m_uploadTreeCurrentRow, OPERATE_FILE, treeOperate_File);
    ui->upload_TreeView->setModel(m_uploadModel);

    // set row and value
    struct TransferTreeViewItem *uploadTree = new TransferTreeViewItem;
    uploadTree->m_dirUrl = filePath;
    uploadTree->m_treeMd5 = md5;
    uploadTree->m_row = m_uploadTreeCurrentRow;
    uploadTree->m_value = value;
    uploadTree->m_treeFileSize = fileSize;
    // push back
    m_uploadTreeVector.push_back(uploadTree);

    m_uploadTreeCurrentRow++;
}

// 服务器端删除文件
void upload::delFile(UserFileInfo *delInfo)
{
    // MessageBox 记录信息
    if(m_message->text() == NULL)
    {
        m_message->setText("删除文件完成，详情信息请点击 Show Details！");
    }
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

    qDebug() << QString(deleteFile);

    // 发送post
    QNetworkReply *reply = manager->post(request, deleteFile);
    connect(reply, &QNetworkReply::readyRead, [=](){
        QByteArray all = reply->readAll();
        ui->textEdit->append(all);

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

            m_message->setDetailedText(m_message->detailedText() + tr("您选择的文件 %1 已经删除！\n").arg(delInfo->m_filename));
        }
        else if("002" == status)
        {
            m_message->setDetailedText(m_message->detailedText() + tr("您选择的文件 %1 删除失败！\n").arg(delInfo->m_filename));
        }
        else
        {
            m_message->setDetailedText(m_message->detailedText() + tr("服务器繁忙, 您选择的文件 %1 删除失败！\n").arg(delInfo->m_filename));
        }
    });

    connect(reply, &QNetworkReply::finished, [=](){
        reply->deleteLater();
        manager->deleteLater();
        if(--m_messageCount == 0)
        {
            m_message->exec();
            m_message->deleteLater();
        }
    });
}

// 发送服务器要共享的文件
void upload::shareFile(UserFileInfo *shareInfo)
{
    //MessageBox 记录信息
    if(m_message->text() == NULL)
    {
        m_message->setText("分享完成，详情信息请点击 Show Details！");
    }
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
        QByteArray all = reply->readAll();
        ui->textEdit->append(all);

        QJsonDocument replyDoc = QJsonDocument::fromJson(all);
        QJsonObject replyObj = replyDoc.object();

        QString status = replyObj.value("code").toString();
        if("001" == status)
        {
            // 添加至共享列表
            QListWidgetItem *item = new QListWidgetItem();
            QIcon icon;
            icon.addFile(getListIcon(shareInfo->m_suffix.toUtf8().data()));
            item->setSizeHint(QSize(ICONWIDTH, ICONHEIGHT));
            item->setText(shareInfo->m_filename);
            item->setIcon(icon);

            // 创建UserFileInfo
            struct UserFileInfo *userShareInfo = new UserFileInfo;
            userShareInfo->m_filename = shareInfo->m_filename;
            userShareInfo->m_username = loginInstance->getUserName();
            userShareInfo->m_md5 = shareInfo->m_md5;
            userShareInfo->m_suffix = shareInfo->m_suffix;
            userShareInfo->m_date = shareDate;
            userShareInfo->m_size = shareInfo->m_size;
            userShareInfo->m_downloadCount = 0;
            userShareInfo->m_isShare = true;
            userShareInfo->m_list = item;

            //qDebug() << "m_shareVector.size:" << m_shareVector.size();
            // 分享容器添加数据
            m_shareVector.push_back(userShareInfo);
            // 分享区添加主题
            ui->myShare_ListWidget->addItem(item);

            m_message->setDetailedText(m_message->detailedText() + tr("文件 %1 分享成功！\n").arg(shareInfo->m_filename));
        }
        else if("003" == status)
        {
            m_message->setDetailedText(m_message->detailedText() + tr("文件 %1 已经被分享过了！\n").arg(shareInfo->m_filename));
        }
        else
        {
            m_message->setDetailedText(m_message->detailedText() + tr("服务器出错，文件 %1 分享失败！\n").arg(shareInfo->m_filename));
        }
    });

    connect(reply, &QNetworkReply::finished, [=](){
        reply->deleteLater();
        manager->deleteLater();
        if(--m_messageCount == 0)
        {
            m_message->exec();
            m_message->deleteLater();
        }
    });
}

void upload::cancelShareFile(UserFileInfo *cancelShareInfo)
{
    //MessageBox 记录信息
    if(m_message->text() == NULL)
    {
        m_message->setText("取消分享完成，详情信息请点击 Show Details！");
    }
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

        QJsonDocument replyDoc = QJsonDocument::fromJson(all);
        QJsonObject replyObj = replyDoc.object();

        QString status = replyObj.value("code").toString();
        if("001" == status)
        {
            ui->myShare_ListWidget->removeItemWidget(cancelShareInfo->m_list);

            // 从vector中取出UserFileInfo
            int cancelShareNumber = m_shareVector.indexOf(cancelShareInfo);
            m_shareVector.remove(cancelShareNumber);

            delete cancelShareInfo->m_list;
            delete cancelShareInfo;

            m_message->setDetailedText(m_message->detailedText() + tr("取消分享文件 %1 成功！\n").arg(cancelShareInfo->m_filename));
        }
        else if("003" == status)
        {
            m_message->setDetailedText(m_message->detailedText() + tr("取消分享文件 %1 失败！\n").arg(cancelShareInfo->m_filename));
        }
        else
        {
            m_message->setDetailedText(m_message->detailedText() + tr("服务器繁忙，取消分享文件 %1 失败！\n").arg(cancelShareInfo->m_filename));
        }
    });

    connect(reply, &QNetworkReply::finished, [=](){
        reply->deleteLater();
        manager->deleteLater();
        if(--m_messageCount == 0)
        {
            m_message->exec();
            m_message->deleteLater();
        }
    });

}

// 查看文件属性
void upload::fileAttribute(UserFileInfo *attributeInfo, int flag)
{
    m_propertyMessage = new QMessageBox(this);
    m_propertyMessage->setText("文件属性");
    m_propertyMessage->setStandardButtons(QMessageBox::Ok);
    m_propertyMessage->setDefaultButton(QMessageBox::Ok);

    if(flag == MAINFILE)
    {
        m_propertyMessage->setInformativeText(QString("文件名：%1\n"
                                              "文件大小：%2\n"
                                              "文件md5：%3\n"
                                              "上传日期：%4\n"
                                              "是否共享：%5").
                                      arg(attributeInfo->m_filename).arg(humanFileSize(attributeInfo->m_size))
                                      .arg(attributeInfo->m_md5).arg(attributeInfo->m_date)
                                      .arg(attributeInfo->m_isShare == true ? "是":"否"));
    }
    else if(flag == SHAREFILE)
    {
        m_propertyMessage->setInformativeText(QString("文件名：%1\n"
                                              "分享来自于：%2\n"
                                              "文件大小：%3\n"
                                              "文件md5：%4\n"
                                              "共享日期：%5\n"
                                              "下载次数：%6").
                                      arg(attributeInfo->m_filename).arg(attributeInfo->m_username)
                                      .arg(humanFileSize(attributeInfo->m_size)).arg(attributeInfo->m_md5)
                                      .arg(attributeInfo->m_date).arg(attributeInfo->m_downloadCount));
    }

    m_propertyMessage->exec();
    m_propertyMessage->deleteLater();
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

// 检查重复下载
void upload::checkDownloadViewItem(const QString filename, const qint64 fileSize, const QString dir, const int value)
{
    if(m_downloadTreeVector.size() > 0)
    {
        QVector<TransferTreeViewItem *>::iterator checkIte = m_downloadTreeVector.begin();
        for (; checkIte != m_downloadTreeVector.end(); checkIte++)
        {
            // 有重复项目
            if((*checkIte)->m_treeFileName == filename)
            {
                // 更改传入下载的行、值
                (*checkIte)->m_value = 0;
            }
        }
    }

    // 用 delegate 实现动态进度条
    QStandardItem *treeEmpty_First = new QStandardItem("*");
    treeEmpty_First->setEditable(false);
    QStandardItem *treeFileName = new QStandardItem(filename);
    treeFileName->setEditable(false);
    QStandardItem *treeFileSize = new QStandardItem(humanFileSize(fileSize));
    treeFileSize->setEditable(false);
    treeFileSize->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
    QStandardItem *treeProgressBar = new QStandardItem("0");
    treeProgressBar->setEditable(false);
    QStandardItem *treeOperate_File = new QStandardItem("");
    treeOperate_File->setEditable(false);

    m_downloadModel->setItem(m_downloadTreeCurrentRow, EMPTY_FIR, treeEmpty_First);
    m_downloadModel->setItem(m_downloadTreeCurrentRow, FILENAME, treeFileName);
    m_downloadModel->setItem(m_downloadTreeCurrentRow, FILESIZE, treeFileSize);
    m_downloadModel->setItem(m_downloadTreeCurrentRow, PROGRESS, treeProgressBar);
    m_downloadModel->setData(m_downloadModel->index(m_downloadTreeCurrentRow, PROGRESS), value);
    m_downloadModel->setItem(m_downloadTreeCurrentRow, OPERATE_FILE, treeOperate_File);
    ui->download_TreeView->setModel(m_downloadModel);

    // set row and value
    struct TransferTreeViewItem *downloadTree = new TransferTreeViewItem;
    downloadTree->m_dirUrl = dir;
    downloadTree->m_treeFileName = filename;
    downloadTree->m_row = m_downloadTreeCurrentRow;
    downloadTree->m_value = value;
    downloadTree->m_treeFileSize = fileSize;
    // push back
    m_downloadTreeVector.push_back(downloadTree);

    m_downloadTreeCurrentRow++;
}

// 发送服务器要下载的文件
void upload::downloadFile(UserFileInfo *downloadInfo, QString dir)
{
    if(dir == NULL)
    {
        qDebug() << "NULL";
        return;
    }

    QString filePath = dir + "/" + downloadInfo->m_filename;

    // 是否重复下载
    checkDownloadViewItem(downloadInfo->m_filename, downloadInfo->m_size, dir);

    // 更新配置文件
    updateConfigFile(DOWNLOADLIST);

    // 传给多线程下载
    emit startRunning(filePath, loginInstance->getAddress(), loginInstance->getUserName(), downloadInfo->m_filename,
                      downloadInfo->m_md5, QString::number(downloadInfo->m_size));

    // progressBar value changing;
    connect(m_worker, &MultiThread::downloadResultReady, [=](QString fileName, int value){
        for (int i = 0; i < m_downloadTreeVector.size(); i++)
        {
            if(m_downloadTreeVector[i]->m_treeFileName == fileName)
            {
                m_downloadModel->setData(m_downloadModel->index(m_downloadTreeVector[i]->m_row, PROGRESS), value);
                break;
            }
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

// 清空文件列表和item项目
void upload::deleteList()
{
    // 清空用户区
    int n = m_vector.size();
    for (int i = 0; i < n; i++)
    {
        UserFileInfo *temp = m_vector.takeFirst();

        delete temp->m_list;
        delete temp;
    }
    // 清空分享区
    int shareNum = m_shareVector.size();
    for (int i = 0; i < shareNum; i++)
    {
        UserFileInfo *temp = m_shareVector.takeFirst();

        delete temp->m_list;
        delete temp;
    }
}

QString upload::humanFileSize(qint64 size)
{
    QStringList units;
    units << "B" << "KB" << "MB" << "GB" << "TB";
    double mod = 1024.0;
    double fileSize = size;
    int i = 0;
    long rest = 0;
    QString result;

    while(fileSize >= mod && i < units.count() - 1)
    {
        rest = (long)fileSize % (long)mod;
        fileSize /= mod;
        i++;
    }
    // 取整数
    int temp = (int)fileSize;
    result = QString::number(temp);

    if(rest > 0)
    {
        result += QString(".") + QString::number(rest).left(2) + QString(" ");
    }

    result += units[i];

    return result;
}

// 更新 m_downloadTreeVector->m_row
void upload::updateCurrentRow(int flag)
{
    QVector<TransferTreeViewItem *> tempVec;

    flag == DOWNLOADLIST ? tempVec = m_downloadTreeVector : tempVec = m_uploadTreeVector;

    QVector<TransferTreeViewItem *>::iterator ite = tempVec.begin();
    for (; ite != tempVec.end(); ite++)
    {
        (*ite)->m_row = tempVec.indexOf(*ite);
    }
}
// 更新配置文件
void upload::updateConfigFile(int flag)
{
    QVector<TransferTreeViewItem *> tempTreeVector;

    flag == DOWNLOADLIST ? tempTreeVector = m_downloadTreeVector : tempTreeVector = m_uploadTreeVector;
    flag == DOWNLOADLIST ? m_config->beginWriteArray("downloadTreeList") : m_config->beginWriteArray("uploadTreeList");

    for (int i = 0; i < tempTreeVector.size(); i++)
    {
        m_config->setArrayIndex(i);
        m_config->setValue("dirUrl", tempTreeVector.at(i)->m_dirUrl);
        m_config->setValue("fileName", tempTreeVector.at(i)->m_treeFileName);
        m_config->setValue("row", tempTreeVector.at(i)->m_row);
        m_config->setValue("progressBarValue", tempTreeVector.at(i)->m_value);
        m_config->setValue("fileSize", tempTreeVector.at(i)->m_treeFileSize);
        m_config->setValue("md5", tempTreeVector.at(i)->m_treeMd5);
    }
    m_config->endArray();
}

void upload::clearItemWidgets()
{
    // 我的文件区创建 selectedItems
    QList<QListWidgetItem *> myFileItems = ui->myFile_ListWidget->selectedItems();
    // 我的分享区创建 selectedItems
    QList<QListWidgetItem *> myShareItems = ui->myShare_ListWidget->selectedItems();
    // 他人的分享区创建 selectedItems
    QList<QListWidgetItem *> otherShareItems = ui->otherShare_ListWidget->selectedItems();

    if(myFileItems.size() > 0)
    {
        for (int i = 0; i < myFileItems.size(); i++)
        {
            myFileItems[i]->setSelected(false);
        }
    }
    if(myShareItems.size() > 0)
    {
        for (int i = 0; i < myShareItems.size(); i++)
        {
            myShareItems[i]->setSelected(false);
        }
    }
    if(otherShareItems.size() > 0)
    {
        for (int i = 0; i < otherShareItems.size(); i++)
        {
            otherShareItems[i]->setSelected(false);
        }
    }
}

void upload::addUploadFileInfo(QString path, QString md5, qint64 size)
{
    checkUploadViewItem(path, md5, size);
}

void upload::uploadListFinish()
{
    // 更新配置文件
    updateConfigFile(UPLOADLIST);
}

void upload::uploadSetProgressBar(QString md5, int value)
{
    for (int i = 0; i < m_uploadTreeVector.size(); i++)
    {
        if(m_uploadTreeVector[i]->m_treeMd5 == md5)
        {
             m_uploadModel->setData(m_uploadModel->index(m_uploadTreeVector[i]->m_row, PROGRESS), value);
             break;
        }
    }
}

void upload::initStyleSheet()
{
    QString styleSheet = "upload * \
    {\
        font-family: Arial;\
        font-size: 20px;\
    }\
    QListWidget#myShare_ListWidget, #otherShare_ListWidget, #myFile_ListWidget\
    {\
        font-size: 16px;\
    }\
    QLabel#myshare_Label, #othershare_Label\
    {\
        font-size: 30px;\
        color: white;\
    }\
    QLabel#uploadFontLabel\
    {\
        font-size: 25px;\
        font-weight: bold;\
        color: white;\
    }\
    QPushButton {\
          border: 2px solid #8f8f91;\
          border-radius: 6px;\
          background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\
                                            stop: 0 #f6f7fa, stop: 1 #dadbde);\
          min-width: 80px;\
    }\
    QPushButton:pressed {\
          background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\
                                            stop: 0 #dadbde, stop: 1 #f6f7fa);\
    }\
    QPushButton:flat {\
          border: none;\
    }\
    QPushButton:default {\
          border-color: navy;\
    }\
    QListView {\
          show-decoration-selected: 1;\
    }\
    QListView::item:alternate {\
          background: #EEEEEE;\
    }\
    QListView::item:selected {\
          border: 1px solid #6a6ea9;\
    }\
    QListView::item:selected:!active {\
          background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\
                                      stop: 0 #ABAFE5, stop: 1 #8588B2);\
    }\
    QListView::item:selected:active {\
          background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\
                                      stop: 0 #6a6ea9, stop: 1 #888dd9);\
    }\
    QListView::item:hover {\
          background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\
                                      stop: 0 #FAFBFE, stop: 1 #DCDEF1);\
    }\
    QMenu\
    {\
          background-color: #e8daef;\
          border: 1px solid black;\
    }\
    QMenu::item\
    {\
          background-color: transparent;\
    }\
    QMenu::item:selected \
    {\
          background-color: #e74c3c;\
    }\
    QProgressBar::chunk {\
          background-color: #05B8CC;\
          width: 20px;\
    }\
    QWidget#smallWidget\
    {\
          border-image: url(:/images/title_bk2.jpg);\
    }";

    this->setStyleSheet(styleSheet);
}

void upload::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap pix(":/images/title_bk.jpg");
    painter.drawPixmap(0, 0, this->width(), this->height(), pix);
    painter.end();
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
