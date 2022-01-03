#ifndef UPLOAD_H
#define UPLOAD_H

#include <QWidget>
#include <QFile>
#include <QByteArray>
#include <QPoint>
#include <QMouseEvent>
#include <QPixmap>
#include <QPainter>
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QListWidgetItem>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QQueue>
#include <QTextCodec>
#include <QDateTime>
#include <QThread>
#include <QProgressBar>
#include <QStandardItemModel>
#include <QTimer>
#include <QDesktopServices>
#include <QProcess>
#include <QSettings>

#include "logininfodata.h"
#include "modifypasswd.h"
#include "multithread.h"
#include "delegate.h"

// 版本号
const QString VERSION = "V1.0";
// 下载文件时传入，判定是主界面下载还是分享下载
const int MAINFILE = 1;
const int SHAREFILE = 2;

// 判断是下载列表还是上传列表
const int DOWNLOADLIST = 1;
const int UPLOADLIST = 2;

// ListWidget 图标尺寸
const int ICONWIDTH = 115;
const int ICONHEIGHT = 110;

// 用户文件属性
struct UserFileInfo{
    QString m_filename;
    QString m_username;
    QString m_md5;
    QString m_suffix;
    QString m_date;
    qint64 m_size;
    int m_downloadCount;
    bool m_isShare;

    QListWidgetItem *m_list;        //存储item地址
};
// 下载列表的 value 和 row
struct TransferTreeViewItem{
    QString m_dirUrl;
    QString m_treeFileName;
    QString m_treeMd5;
    int m_row;
    int m_value;
    qint64 m_treeFileSize;
};

// Download_TreeView 列名称
enum TreeViewColumn{
    EMPTY_FIR,                      // 空行
    FILENAME,                       // 文件名称
    FILESIZE,                       // 文件大小
    PROGRESS,                       // 进度
    OPERATE_FILE,                   // 操作
    COL_COUNT                       // 总数
};

namespace Ui {
class upload;
}

class upload : public QWidget
{
    Q_OBJECT

public:
    explicit upload(QWidget *parent = nullptr);
    ~upload();

    void initUploadWindow();                        // 初始化ui界面
    void initStyleSheet();                          // 初始化qss
    void initThread();                              // 初始化线程
    void addMenuAction();                           // 初始化菜单栏
    void readTransferConfigure();                   // 读取下载区配置

    void showFileList();                            // 初始化用户文件到列表

    void executeAction(QAction *action);            // 根据菜单执行动作

    void addCommonFileList(QString filename,         // 添加文件到列表
                           QString md5, int num, qint64 fileSize, QString fileDate);

    void addShareFileList(QString username,          // 添加共享文件到列表
                          QString filename, QString md5, qint64 size, int downloadCount, QString fileDate);


    void checkUploadViewItem(const QString filePath, const QString md5,
                             const qint64 fileSize, const int value = 0);               // 检查重复上传
    void checkDownloadViewItem(const QString filename, const qint64 fileSize,
                               const QString dir, const int value = 0);                 // 检查重复下载

    void uploadFileData();                                      // 发送服务器上传文件
    void downloadFile(UserFileInfo *downloadInfo, QString dir); // 发送服务器要下载的文件
    void delFile(UserFileInfo *delInfo);                        // 发送服务器要删除的文件
    void shareFile(UserFileInfo *shareInfo);                    // 发送服务器要共享的文件
    void cancelShareFile(UserFileInfo *cancelShareInfo);        // 发送服务器要取消分享的文件
    void fileAttribute(UserFileInfo *attributeInfo, int flag);  // 文件属性

    QString getListIcon(char *suffix);              // 根据后缀获取列表图标

    void deleteList();                              // 刷新时清理用户item

    QString humanFileSize(qint64 size);             // 转化文件大小

    void updateApplication();                       // 更新软件
    void updateCurrentRow(int flag);                // 下载区删除文件时更新 m_row
    void updateConfigFile(int flag);                // 更新配置文件

    void clearItemWidgets();                        //取消被选中状态(多个 QListWidget 切换时可能发生多个选中状态)

signals:
    void switchUser();                              // 切换用户

    void startRunning(QString filePath, QString addr, QString username,
                      QString filename, QString md5, QString size); // 触发线程

    void uploadStartRunning(QStringList list, QString username, QString addr);

private slots:
    void addUploadFileInfo(QString path, QString md5, qint64 size);
    void uploadListFinish();
    void uploadSetProgressBar(QString md5, int value);

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

private:
    Ui::upload *ui;

    QPoint m_point;

    QByteArray mainFileList;

    // 判断窗口移动还是缩放
    bool m_bSize = false;
    bool m_bMove = false;
    bool m_bSizeOrMove = true;

    // 单例模式
    LoginInfoData *loginInstance;

    // 我的文件区主界面模板
    QVector<UserFileInfo *> m_vector;
    // 分享区文件模板
    QVector<UserFileInfo *>m_shareVector;

    // 菜单栏
    // ---------图标设置栏----------
    QMenu *m_iconMenu;
    QAction *m_userInfoAction;
    QAction *m_updateAction;
    QAction *m_modifyPwdAction;
    QAction *m_switchUserAction;
    QAction *m_exitLoginAction;
    // ---------用户区-------------
    QMenu *m_userMenu;
    QAction *m_downloadAction;
    QAction *m_shareAction;
    QAction *m_delAction;
    QAction *m_propertyAction;
    // ---------分享区-------------
    QMenu *m_myShareMenu;
    QMenu *m_otherShareMenu;
    QAction *m_myShareCancelAction;
    QAction *m_myShareDownloadAction;
    QAction *m_otherShareDownloadAction;
    // ---------空白区-------------
    QMenu *m_blankMenu;
    QAction *m_flushAction;
    QAction *m_uploadFileAction;

    int m_messageCount;                         // messagebox 接收 details 的数量,在合适的时候弹出 messageBox
    QMessageBox *m_message;                     // 自定义消息 messagebox
    QMessageBox *m_propertyMessage;

    static int m_fileNumber;

    ModifyPasswd *m_modifyPwd;                  // 修改密码

    QThread *m_transferThread;                  // 线程
    MultiThread *m_worker;

    // model-view-delegate
    delegate *m_downloadDelegate;
    delegate *m_uploadDelegate;
    QStandardItemModel *m_downloadModel;
    QStandardItemModel *m_uploadModel;
    // storage DownloadTreeView*
    QVector<TransferTreeViewItem *> m_downloadTreeVector;
    QVector<TransferTreeViewItem *> m_uploadTreeVector;

    int m_downloadTreeCurrentRow;             // QTreeView 行数
    int m_uploadTreeCurrentRow;             // QTreeView 行数

    // config.ini
    QSettings *m_config;
};

#endif // UPLOAD_H
