#ifndef MULTITHREAD_H
#define MULTITHREAD_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFileInfo>
#include <QProgressBar>
#include <QTimer>
#include <QMessageBox>
#include <QHttpMultiPart>
#include <QListWidgetItem>

class MultiThread : public QObject
{
    Q_OBJECT
public:
    explicit MultiThread(QObject *parent = nullptr);
    ~MultiThread();

    QString getMd5(const QString path);                             // 获取文件md5
    QString getSuffix(const QString suffix);                        // 根据文件后缀提供Content-type

signals:
    void downloadResultReady(QString fileName, int progressValue);                   // 向外界发送下载结果


    void sendUploadProgress(QString filePath, int progressValue);   // 发送上传过程
    void uploadResultReady(QString filename, QString md5, int num,  // 向外界发送上传结果
                           qint64 fileSize, QString fileDate);

    void uploadFileInfo(QString filePath, QString md5, qint64 fileSize);
    void uploadFinish();

public slots:
    void startDownload(QString filePath, QString addr, QString username,
                       QString filename, QString md5, QString size);   //耗时下载操作

    void startUpload(QStringList list, QString username, QString addr);

};

#endif // MULTITHREAD_H
