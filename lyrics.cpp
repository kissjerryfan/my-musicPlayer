#include "lyrics.h"
#include "dialog.h"

//获取歌词文本
QList<QString> Lyrics::getListLyricsText() const
{
    return listLyricsText;
}

//获取歌曲时间
QList<int> Lyrics::getListLyricsTime() const
{
    return listLyricsTime;
}

//传入路径
Lyrics::Lyrics(QString lyricsPath)
{
    this->lyricsPath = lyricsPath;
}
Lyrics::Lyrics()
{
}

//读取歌词文件
bool Lyrics::readLyricsFile(QString lyricsPath)
{
    QFile file(lyricsPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        listLyricsText.clear();
        return false;
    }
    QString line="";
    while((line=file.readLine())>0){
        qDebug()<<line;
        analysisLyricsFile(line);
    }
    return true;
}

//解析歌词文件
bool Lyrics::analysisLyricsFile(QString line)
{
    if(line == NULL || line.isEmpty()){
        qDebug()<<"thie line is empty!";
        return false;
    }
    QRegularExpression regularExpression("\\[(\\d+)?:(\\d+)?(\\.\\d+)?\\](.*)?");
    int index = 0;
    QRegularExpressionMatch match;
    match = regularExpression.match(line, index);
    if(match.hasMatch()) {
        QString currentText =QString::fromStdString(match.captured(4).toStdString());      /*   获取歌词文本*/
        qDebug()<<currentText;
        listLyricsText.push_back(currentText);
        return true;
    }
    return false;
}
