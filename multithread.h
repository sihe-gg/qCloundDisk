#ifndef MULTITHREAD_H
#define MULTITHREAD_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>


class MultiThread : public QObject
{
    Q_OBJECT
public:
    explicit MultiThread(QObject *parent = nullptr);

signals:
    void resultReady(const QString &str);     // 向外界发送结果

public slots:
    void startDownload(QString filePath, QString addr, QString username, QString filename, QString md5, QString size);   //耗时下载操作

};

#endif // MULTITHREAD_H
