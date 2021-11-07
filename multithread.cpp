#include "multithread.h"

MultiThread::MultiThread(QObject *parent) : QObject(parent)
{

}

void MultiThread::startDownload(int row, QString filePath, QString addr, QString username,
                                QString filename, QString md5, QString size)
{
    // 定时器用来传递进度条
    QTimer *timer = new QTimer();
    timer->setInterval(1000);

    QFile *file = new QFile(filePath);
    if(!file->open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(NULL, "创建文件" , "创建文件失败，请重试！");

        file->deleteLater();
        return;
    }

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QUrl url(QString("http://%1/download").arg(addr));
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "text/plain");

    // 下载格式:username filename md5 size 最后空格
    QByteArray downloadData = QString("%1 %2 %3 %4 ").arg(username).
            arg(filename).arg(md5).arg(size).toUtf8();

    qDebug() << "download:" << downloadData;

    QNetworkReply *reply = manager->post(request, downloadData);

    timer->start();
    connect(reply, &QNetworkReply::readyRead, [=](){
        qint64 size;
        while((size = reply->bytesAvailable()) > 0)
        {
            QByteArray all = reply->readAll();
            file->write(all);
        }
    });

    connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesReceived, qint64 bytesTotal){
        if(bytesTotal == 0)
        {
            return;
        }

        bytesTotal = size.toLong();
        qint64 downloadPercentage = (bytesReceived * 100) / size.toLong();

        emit resultReady(row, downloadPercentage);
    });

    // 下载完成释放资源及递归调用下载
    connect(reply, &QNetworkReply::finished, [=](){
        // 延迟关闭定时器
        timer->singleShot(1000, this, [=](){
            if(timer->isActive())
            {
                qDebug() << "timer stop";
                timer->stop();
            }

            file->close();
            file->deleteLater();
            timer->deleteLater();
            reply->deleteLater();
            manager->deleteLater();

            qDebug() << "下载完成，释放资源完成";
        });
    });

    // 下载错误检查
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            [=](QNetworkReply::NetworkError code)
    {
        qDebug() << "code :" << code;
        if(code == QNetworkReply::UnknownServerError)
        {
            if(timer->isActive())
            {
                qDebug() << "timer stop";
                timer->stop();
            }
            file->close();
            file->deleteLater();
            reply->deleteLater();
            manager->deleteLater();
            return;
        }
    });

    connect(timer, &QTimer::timeout, [=](){
        emit timerSentTimeOut(row);
    });
}
