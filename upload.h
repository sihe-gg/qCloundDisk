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

#include "logininfodata.h"
#include "modifypasswd.h"

// 版本号
const QString VERSION = "V1.0";
// 下载文件时传入，判定是主界面下载还是分享下载
const int MAINFILE = 1;
const int SHAREFILE = 2;

// ListWidget图标尺寸
const int ICONWIDTH = 115;
const int ICONHEIGHT = 110;

// 用户文件属性
struct userFileInfo{

    QString m_filename;
    QString m_username;
    QString m_md5;
    QString m_suffix;
    QString m_date;
    long m_size;
    int m_downloadCount;
    bool m_isShare;

    QListWidgetItem *m_list;        //存储item地址
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
    void initUserFileList();                        // 初始化用户文件到列表
    void addMenuAction();                           // 初始化菜单栏
    void executeAction(QAction *action);            // 根据菜单执行动作

    void uploadFileData();                          // 批量上传文件数据

    void showFileList(QByteArray data);             // 展示文件到列表上


    void addCommonFileList(QString filename,         // 添加文件到列表
                           QString md5, int num, long fileSize, QString fileDate);

    void addShareFileList(QString username,          // 添加共享文件到列表
                          QString filename, QString md5, long size, int downloadCount, QString fileDate);

    void downloadFile(QList<QListWidgetItem *> fileItems,     // 发送服务器要下载的文件
                      QString dir, int flag);
    void delFile(QList<QListWidgetItem *> fileItems);         // 发送服务器要删除的文件
    void shareFile(QList<QListWidgetItem *> fileItems);       // 发送服务器要共享的文件
    void cancelShareFile(QList<QListWidgetItem *> fileItems); // 发送服务器要取消分享的文件
    void fileAttribute(QList<QListWidgetItem *> fileItems, int flag);   // 文件属性

    void updateApplication();                       // 更新软件

    void getUploadFilePath();                       // 获取文件路径
    QString getListIcon(char *suffix);              // 根据后缀获取列表图标
    QString getMd5(const QString path);             // 获取文件md5
    QString getSuffix(const QString suffix);        // 根据文件后缀提供Content-type

    void deleteList();                              // 刷新时清理用户item
    void paintVector();                             // 临时打印容器信息

signals:
    void switchUser();                              // 切换用户

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);

private:
    Ui::upload *ui;

    QPoint m_point;

    // 判断窗口移动还是缩放
    bool m_bSize = false;
    bool m_bMove = false;
    bool m_bSizeOrMove = true;

    // 单例模式
    LoginInfoData *loginInstance;

    // 批量上传区文件模板
    QVector<QListWidgetItem *> m_uploadVector;
    // 我的文件区主界面模板
    QVector<userFileInfo *> m_vector;
    // 分享区文件模板
    QVector<userFileInfo *>m_shareVector;

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
    QAction *m_flushAction;
    // ---------分享区-------------
    QMenu *m_myShareMenu;
    QMenu *m_otherShareMenu;
    QAction *m_myShareCancelAction;
    QAction *m_myShareDownloadAction;
    QAction *m_otherShareDownloadAction;
    // ---------上传区-------------
    QMenu *m_uploadMenu;
    QAction *m_delFilePathAction;

    QMessageBox *m_message;                     // 自定义messagebox

    static int m_fileNumber;

    ModifyPasswd *m_modifyPwd;

};

#endif // UPLOAD_H
