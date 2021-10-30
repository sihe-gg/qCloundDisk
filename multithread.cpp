#include "multithread.h"

MultiThread::MultiThread(QObject *parent) : QObject(parent)
{

}

void MultiThread::startDownload(QString filePath, QString addr, QString username, QString filename, QString md5, QString size)
{
    qDebug() << "multi threadId:" << QThread::currentThreadId();

    QFile *file = new QFile(filePath);
    if(!file->open(QIODevice::WriteOnly))
    {
        //QMessageBox::warning(this, "创建文件" , "创建文件失败，请重试！");

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

    connect(reply, &QNetworkReply::readyRead, [=](){
        qint64 size;
        while((size = reply->bytesAvailable()) > 0)
        {
            QByteArray all = reply->readAll();
            qDebug() << "all :" << all;
            //ui->textEdit->append(all);

            file->write(all);
        }
    });

    connect(reply, &QNetworkReply::downloadProgress, [=](qint64 bytesReceived, qint64 bytesTotal){
        if(bytesTotal == 0)
            return;

        //bytesTotal == downloadInfo->m_size
        //qint64 downloadPercentage = (bytesReceived * 100) / size.toLong();
        //ui->status_Label->setText(tr("文件: %1 正在下载: %2\%").arg(downloadInfo->m_filename).arg(QString::number(downloadPercentage)));
    });

    // 下载完成释放资源及递归调用下载
    connect(reply, &QNetworkReply::finished, [=](){
        //m_message->setDetailedText(m_message->detailedText() + tr("文件 %1 下载成功！\n").arg(downloadInfo->m_filename));

        file->close();
        file->deleteLater();
        reply->deleteLater();
        manager->deleteLater();
        qDebug() << "下载完成，释放资源完成";

        emit resultReady("hello");

        // 下载完成递归调用下载
        //downloadFile(fileItems, dir, flag);
    });

    // 下载错误检查
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            [=](QNetworkReply::NetworkError code)
    {
        qDebug() << "code :" << code;
        if(code == QNetworkReply::UnknownServerError)
        {
           // m_message->setDetailedText(m_message->detailedText() + tr("连接服务器失败，文件 %1 下载失败！\n").arg(downloadInfo->m_filename));

            file->close();
            file->deleteLater();
            reply->deleteLater();
            manager->deleteLater();
            return;
        }
    });

}
