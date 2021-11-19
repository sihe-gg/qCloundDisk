#include "multithread.h"

MultiThread::MultiThread(QObject *parent) : QObject(parent)
{
    qDebug() << "I'm in";
}

MultiThread::~MultiThread()
{
    qDebug() << "I'm out";
}

void MultiThread::startDownload(QString filePath, QString addr, QString username,
                                QString filename, QString md5, QString size)
{
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

        emit downloadResultReady(filename, downloadPercentage);
    });

    // 下载完成释放资源及递归调用下载
    connect(reply, &QNetworkReply::finished, [=](){
        file->close();
        file->deleteLater();
        reply->deleteLater();
        manager->deleteLater();

        qDebug() << "下载完成，释放资源完成";
    });

    // 下载错误检查
    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            [=](QNetworkReply::NetworkError code)
    {
        qDebug() << "code :" << code;
        if(code == QNetworkReply::UnknownServerError)
        {

            file->close();
            file->deleteLater();
            reply->deleteLater();
            manager->deleteLater();
            return;
        }
    });


}

void MultiThread::startUpload(QStringList list, QString username, QString addr)
{
    while(list.size() > 0)
    {
        // 准备数据
        QString filePath = list.takeFirst();
        QString md5 = getMd5(filePath);

        qDebug() << filePath;
        QFileInfo info(filePath);
        QString uploadDate = QDateTime::currentDateTime().toString("yyyy-MM-dd&HH:mm:ss");
        QString type = getSuffix(info.suffix());
        if(type == " ")
        {
            return;
        }

        QString dispositionData = QString("form-data; name=%1; filename=%2; md5=%3; size=%4; date=%5").
                arg(username).arg(info.fileName()).arg(md5).arg(info.size()).arg(uploadDate);
        QUrl url((QString("http://%1/upload").arg(addr)));

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

        emit uploadFileInfo(filePath, md5, info.size());
        //取回服务器数据
        connect(reply, &QNetworkReply::readyRead, [=](){
            //显示记录
            QByteArray all = reply->readAll();

            // 判断上传是否成功
            QString buf = all;

            if(strncmp("成功！", buf.toUtf8().data(), strlen("成功！")) == 0)
            {
                // 将上传的文件添加到主界面
                emit uploadResultReady(info.fileName(), getMd5(filePath), 0, info.size(), uploadDate);
            }

        });

        //进度条处理
        connect(reply, &QNetworkReply::uploadProgress, [=](qint64 bytesSent, qint64 bytesTotal){
            if(bytesTotal == 0)
                return;

            emit sendUploadProgress(md5, (bytesSent * 100 / bytesTotal));
        });

        // 上传文件完成
        connect(reply, &QNetworkReply::finished, [=](){
            // 清理内存，关闭文件
            file->close();
            file->deleteLater();
            reply->deleteLater();
            manage->deleteLater();
            multiPart->deleteLater();

            emit uploadFinish();
            qDebug() << "上传完成，释放完成";
        });
    }

}

QString MultiThread::getMd5(const QString path)
{
    // 获取文件md5
    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile file(path);
    file.open(QIODevice::ReadOnly);

    hash.addData(&file);

    QByteArray result = hash.result().toHex();

    return result;
}

QString MultiThread::getSuffix(const QString suffix)
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

