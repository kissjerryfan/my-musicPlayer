/*
#ifndef KUGOU_H
#define KUGOU_H

#include <QObject>

class kugou : public QObject
{
    Q_OBJECT
public:
    explicit kugou(QObject *parent = nullptr);

signals:

public slots:
};

#endif // KUGOU_
*/

#ifndef KUGOU_H
#define KUGOU_H

#include <QObject>
#include<QMediaPlaylist>
#include <QMediaPlayer>
#include<QPaintEvent>
#include<QNetworkAccessManager>
#include<QNetworkReply>
#include<QNetworkRequest>
#include<QJsonValue>
#include<QJsonArray>
#include<QJsonDocument>
#include<QJsonObject>

class KuGou : public QObject
{
    Q_OBJECT
public:
    explicit KuGou(QObject *parent = 0);

    void parseJson_getAlbumID(QString json);
    void parseJson_getplay_url(QString json);

    void search(QString str);
signals:
    void nameAdd(QString name);
    void mediaAdd(QString urlStr);
    void lrcAdd(QString lrcStr);
public slots:
    void replyFinished(QNetworkReply *reply);
    void replyFinished2(QNetworkReply *reply);
private:
    QNetworkAccessManager* network_manager;
    QNetworkRequest* network_request;
    QNetworkAccessManager* network_manager2;
    QNetworkRequest* network_request2;

    QString hashStr;
    QString album_idStr;
};

#endif // KUGOU_H
