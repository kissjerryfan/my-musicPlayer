/*
#include "kugou.h"

kugou::kugou(QObject *parent) : QObject(parent)
{

}
*/

#include "kugou.h"

KuGou::KuGou(QObject *parent) : QObject(parent)
{
    network_manager = new QNetworkAccessManager();
    network_request = new QNetworkRequest();
    network_manager2 = new QNetworkAccessManager();
    network_request2 = new QNetworkRequest();
    connect(network_manager2, &QNetworkAccessManager::finished, this, &KuGou::replyFinished2);
    connect(network_manager, &QNetworkAccessManager::finished, this, &KuGou::replyFinished);
}

void KuGou::replyFinished(QNetworkReply *reply)        //发送搜索请求完成，接受到信息，然后进行解析
{
    //获取响应的信息，状态码为200表示正常
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    //无错误返回
    if(reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();  //获取字节
        QString result(bytes);  //转化为字符串
        parseJson_getAlbumID(result);  //自定义方法，解析歌曲数据
    }
    else
    {
        //处理错误
        qDebug()<<"处理错误";
    }
}
void KuGou::replyFinished2(QNetworkReply *reply)       //发送搜索请求完成，接受到信息，然后进行解析         通过歌曲ID搜索
{
    //获取响应的信息，状态码为200表示正常
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    //无错误返回
    if(reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();  //获取字节
        QString result(bytes);  //转化为字符串

        parseJson_getplay_url(result);  //自定义方法，解析歌曲数据
    }
    else
    {
        //处理错误
        qDebug()<<"处理错误";
    }
}

void KuGou::parseJson_getAlbumID(QString json)     //解析接收到的歌曲信息，得到歌曲ID
{
    QByteArray byte_array;
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(byte_array.append(json), &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject rootObj = parse_doucment.object();
            if(rootObj.contains("data"))
            {
                QJsonValue valuedata = rootObj.value("data");
                if(valuedata.isObject())
                {
                    QJsonObject valuedataObject = valuedata.toObject();
                    if(valuedataObject.contains("lists"))
                    {
                        QJsonValue valueArray = valuedataObject.value("lists");
                        if (valueArray.isArray())
                        {
                            QJsonArray array = valueArray.toArray();
                            int size = array.size();
                            for(int i = 0;i < size;i++)
                            {
                                QJsonValue value = array.at(i);
                                if(value.isObject())
                                {
                                    QJsonObject object = value.toObject();
                                    if(object.contains("AlbumID"))
                                    {
                                        QJsonValue AlbumID_value = object.take("AlbumID");
                                        if(AlbumID_value.isString())
                                        {
                                            album_idStr = AlbumID_value.toString();             //歌曲ID信息
                                        }
                                    }
                                    if(object.contains("FileHash"))
                                    {
                                        QJsonValue FileHash_value = object.take("FileHash");
                                        if(FileHash_value.isString())
                                        {
                                            hashStr = FileHash_value.toString();                //hash
                                        }
                                    }

                                    //通过歌曲ID发送请求，得到歌曲url和歌词
                                    QString KGAPISTR1 = QString("https://wwwapi.kugou.com/yy/index.php?r=play/getdata&hash=275B12560EF5C29AAB19537F14FB055E")
                                            .arg(hashStr).arg(album_idStr);

                                    network_request2->setUrl(QUrl(KGAPISTR1));
                                    network_manager2->get(*network_request2);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        qDebug()<<json_error.errorString();
    }
}

void KuGou::parseJson_getplay_url(QString json)        //解析得到歌曲
{
    QByteArray byte_array;
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(byte_array.append(json), &json_error);
    if(json_error.error == QJsonParseError::NoError)
    {
        if(parse_doucment.isObject())
        {
            QJsonObject rootObj = parse_doucment.object();
            if(rootObj.contains("data"))
            {
                QJsonValue valuedata = rootObj.value("data");
                if(valuedata.isObject())
                {
                    QJsonObject valuedataObject = valuedata.toObject();
                    QString play_urlStr("");
                    if(valuedataObject.contains("play_url"))
                    {
                        QJsonValue play_url_value = valuedataObject.take("play_url");
                        if(play_url_value.isString())
                        {
                            play_urlStr = play_url_value.toString();                    //歌曲的url

                            if(play_urlStr!="")
                            {
                                emit mediaAdd(play_urlStr);
                            }

                        }
                    }
                    if(valuedataObject.contains("audio_name"))
                    {
                        QJsonValue play_name_value = valuedataObject.take("audio_name");
                        if(play_name_value.isString())
                        {
                            QString play_name = play_name_value.toString();                //歌曲名字
                            if(play_urlStr!="")
                            {
                                emit nameAdd(play_name);
                            }

                        }
                    }
                    if(valuedataObject.contains("lyrics"))                                  //lrc
                    {
                        QJsonValue play_url_value = valuedataObject.take("lyrics");
                        if(play_url_value.isString())
                        {
                            QString play_lrcStr = play_url_value.toString();
                            if(play_urlStr!="")
                            {
                                if(play_lrcStr != "")
                                {
                                    emit lrcAdd(play_lrcStr);
                                }
                                else
                                {
                                    emit lrcAdd("没有歌词");
                                }
                            }

                        }
                    }
                }
            }
        }
    }
}
void KuGou::search(QString str)
{
    //发送歌曲搜索请求
    QString KGAPISTR1 = QString("http://songsearch.kugou.com/song_search_v2?callback=jQuery191034642999175022426_1489023388639&keyword="
                        "%1&page=1&pagesize=30&userid=-1&clientver=&platform=WebFilter&tag=em&filter=2&iscorrection=1&privilege_filter=0&_=1489023388641").arg(str);


    network_request->setUrl(QUrl(KGAPISTR1));
    network_manager->get(*network_request);
}
