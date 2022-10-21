
#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QMediaPlayer>
#include "lrc_widget.h"
#include "kugou.h"
#include "lyrics.h"
#include <QListWidgetItem>
#include <vector>
#include "lyricwidget.h"
#include <QTableWidgetItem>
#include <QSystemTrayIcon>
#include <QVector>
#include <QMovie>
#include <QSize>

using namespace std;


namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

    //储存所有歌词
    vector<LyricLine> lines;
public:
    explicit Dialog(QWidget *parent = nullptr);
    void get_lrcStrTime();
    QString settime(int time);


    //将歌词文件的内容处理为歌词结构的QList
    bool process(QString filePath);
    //根据时间找到对应位置的歌词
    int getIndex(qint64 position);
    //显示当前播放进度的歌词
    void showcontent(qint64 position);
    //根据下标获得歌词内容
    QString getLyricText(int index);
    //清空歌词Label
    void clear();
    //随着歌曲播放更新歌词
    void updateSongLrc(qint64 position);
    //更新歌词列表
    void updateSongLyclist();
    ~Dialog();
signals:
    //void mySignal();  //该信号用于界面跳转
private slots:
    //播放键
    void on_play_Btn_clicked();

    //静音键
    void on_muteButton_clicked();

    //控制音量变化
    void on_volumeControl_valueChanged(int value);

    //从本地添加歌曲到本地歌曲列表
    void on_add_Btn_clicked();

    //播放时数据的变化
    void on_player_Data_change();

    //播放位置随着时间变化
    void on_player_position_change(qint64);

    //播放进度条
    void on_seekSlider_sliderReleased();

    //播放模式的选择
    void on_playPattern_Btn_clicked();

    //下一首歌曲
    void on_next_Btn_clicked();

    //上一首歌曲
    void on_last_Btn_clicked();

    //媒体状态改变
    void on_mediaStatusChanged(QMediaPlayer::MediaStatus status);

    //关闭界面
    void on_close_Btn_clicked();

    //界面最小化
    void on_min_Btn_clicked();

    //设置界面的背景图
    void paintEvent(QPaintEvent *event);

    //让标题滚动显示
    void doMoveTitle();

    //双击播放
    void on_list_doubleClicked(const QModelIndex &index);

    //音量的实时显示
    void on_volumeValue_linkActivated();

    //改变音量
    void changeVolume();

    //获取文件路径
    QStringList getFileNames(const QString &path);

    //歌词显示按钮
    void on_LycButton_cliked();

    //激活最小化至托盘功能
    void on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason);

    //按下空格键暂停/播放
    void keyPressEvent(QKeyEvent *event);

    //void fullscreen();

    //void halfscreen();

    //void show_slot();(界面跳转槽函数)

    /*****************************************************************
     * QNetworkAccessManager类用于协调网络操作，在QNetworkRequest类发起一个*
     * 网络请求后，QNetworkAccessManager类负责发送网络，创建网络相应。       *
     *                                                               *
     * QNetworkRequest类通过一个URL地址发起网络协议请求，也保存网络请求的信息，*
     * 目前支持HTTP，FTP，和局部文件URLs的下载或上传。                      *
    *****************************************************************/

    //以字符串的形式获取解析到的数据
    void replyFinished(QNetworkReply *reply);

    //解析api借口返回的json
    void parseJson(QString json);

    //搜索
    void search(QString str);

    //以字符串的形式获取解析到的数据
    void replyFinished2(QNetworkReply *reply);

    //拆分解析到的信息
    void parseJsonSongInfo(QString json);

    void on_tableWidget_cellDoubleClicked(int row, int column);

private:
    Ui::Dialog *ui;
    bool isPlay;
    bool isMute;
    bool nowIsFirst;
    int music_position;
    int music_volume;
    int playPattern;
    int titleweizhi;
    int lrcindex;
    int nowLrcListRow;
    int lrcListFirstAdd;
    int m_PlayRow;
    int lyricsID = 0;
    QMediaPlaylist* playList;
    QMediaPlayer* player;
    QMediaPlayer m_mediaPlayer;
    QString titleStr;
    QString m_playPath;
    QString MusicPath="/Qt1/MyPlayer01/musics";
    lrc_widget* myLrc_widget;
    QStringList lrcTime;
    QStringList lrcStr;
    QStringList lrcList;
    QStringList fileList;//文件列表
    KuGou* KuGouSearch;
    Lyrics lyrics;
    lyricwidget *Lyric=new lyricwidget(this);//创建歌词窗口指针
    QSystemTrayIcon *mSysTrayIcon;//实现最小化功能用到的指针
    //添加类成员
    QNetworkAccessManager * network_manager;
    QNetworkRequest * network_request;
    //类成员添加
    QNetworkAccessManager * network_manager2;
    QNetworkRequest * network_request2;
    QVector<QString> songname_original; //歌曲名
    QVector<QString> singername;        //歌手
    QVector<QString> album_name;        //专辑
    QVector<int> duration;              //时间
    QVector<QString> m_Vectorlist;      //保存hash
    QVector<QString> m_ID;              //保存album_id
    QMovie *movie;  //添加动态图
    QSize size;     //设置动态图大小
    QMovie *movie1;  //添加动态图
    QSize size1;     //设置动态图大小
};

#endif // DIALOG_H

