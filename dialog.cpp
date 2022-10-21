#include "dialog.h"
#include "ui_dialog.h"
#include "lrc_widget.h"
#include "lyrics.h"
#include"lyricwidget.h"
#include "login.h"

#include <QDebug>
#include <QMediaPlaylist>
#include <QMessageBox>//提示窗口
#include <QFileInfo>
#include <QTimer>
#include <QTime>
#include <QMediaMetaData>
#include <QPainter>
#include <QUrl>
#include <QString>
#include <QStandardPaths>//文件路径
#include <QFileDialog>//调用本地添加文件窗口
#include <QSystemTrayIcon>
#include <QDesktopWidget>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    isPlay = false;                                             //正在播放为false
    isMute = false;                                             //是否静音为false
    music_position = 0;                                         //歌曲播放进度为0
    music_volume = 50;                                          //初始化音量为50
    playPattern = 0;                                            //播放顺序
    playList = new QMediaPlaylist;                              //播放列表
    playList->setPlaybackMode(QMediaPlaylist::Sequential);      //初始化播放顺序为顺序播放

    player = new QMediaPlayer;
    setMinimumSize(this->width(),this->height());//设置窗体大小不能进行改变
    setMaximumSize(this->width(),this->height());
    setWindowFlags(Qt::FramelessWindowHint);//无边框

    player->setVolume(music_volume);

    ui->volumeControl->setMinimum(0);                           //音量的进度条
    ui->volumeControl->setMaximum(100);

    ui->volumeControl->setValue(music_volume);

    //connect(player,SIGNAL(metaDataAvailableChanged(bool)),this,SLOT(on_player_Data_change()));
    connect(player,SIGNAL(positionChanged(qint64)),this,SLOT(on_player_position_change(qint64)));
    connect(player,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this,SLOT(on_mediaStatusChanged(QMediaPlayer::MediaStatus)));
    nowIsFirst = true;


    myLrc_widget = new lrc_widget();

    QPalette pac;//创建调色板
    pac.setColor(QPalette::WindowText,QColor(0,0,0,0));
    ui->list -> setFont(QFont("Times",12,QFont::Bold));
    ui->list -> setPalette(pac);
    ui->list ->setStyleSheet("background: rgba(0,0,0,0);"
                             "background-color: rgb(255, 0, 0, 0);"
                             "border: 1px solid black;"
                             "margin:10px 10px 10px 10px;");
    ui->Lrc_list->setFont(QFont("Times",15,QFont::Bold));
    ui->Lrc_list->setPalette(pac);
    ui->Lrc_list ->setStyleSheet("background: rgba(0,0,0,0);"
                             "background-color: rgb(255, 0, 0, 0);"
                             "border: 1px solid black;"
                             "margin:10px 10px 10px 10px;");

    ui->time_label->setText("00:00:00");
    ui->now_time_label->setText("00:00:00");

    QTimer* moveTitle = new QTimer(this);           //计时器用来移动标题
    titleweizhi = 0;

    connect(moveTitle,SIGNAL(timeout()),this,SLOT(doMoveTitle()));
    connect(this->ui->close_Btn,SIGNAL(clicked()),this,SLOT(on_close_Btn_clicked()));
    connect(this->ui->min_Btn,SIGNAL(clicked()),this,SLOT(on_min_Btn_clicked()));
    moveTitle->start(1000);


    ui->play_Btn->setToolTip(QString("播放"));
    ui->playPattern_Btn->setToolTip(QString("顺序播放"));
    ui->add_Btn->setToolTip(QString("添加"));
    ui->lrcList_label->setFont(QFont("宋体",18,QFont::Bold));
    ui->nowplay_label->setFont(QFont("宋体",10,QFont::Normal));
    ui->title_label->setFont(QFont("宋体",20,QFont::Normal));
    titleStr = ui->title_label->text();

    connect(player,SIGNAL(durationChanged(qint64)),this,SLOT(on_player_Data_change()));

    ui->logo_label->setScaledContents(true);
    ui->logo_label->setGeometry(0,0,31,31);
    ui->logo_label->setPixmap(QPixmap("/Qt1/MyPlayer01/images/logo.png"));//在左上角显示一个logo的图片


    connect(ui->volumeControl,&QSlider::valueChanged,this,&Dialog::on_volumeValue_linkActivated);//实时音量数字显示


    fileList=getFileNames(this->MusicPath);//获取文件夹下所有音乐文件
    //qDebug()<<"所有音乐 \n"<<fileList;

    QString songnames;
    QFileInfo a;
    for(int i=0;i<fileList.size();i++)
    {
        QString filename=fileList.at(i);//第i个文件
        a = QFileInfo(filename);
        songnames = a.fileName();
        ui->list->addItem(songnames);//添加歌曲名到界面的歌曲列表
        playList->addMedia(QUrl::fromLocalFile(MusicPath+"/"+filename));
    }
    qDebug()<<"原来列表媒体数"<<playList->mediaCount();

    //playList->setCurrentIndex(0);//在播放列表位置激活媒体内容

    player->setPlaylist(playList);

    connect(ui->LycButton,&QPushButton::clicked,this,&Dialog::on_LycButton_cliked);//显示歌词

    connect(player,&QMediaPlayer::positionChanged,this,&Dialog::updateSongLrc);//随时间变化歌词

    connect(playList,&QMediaPlaylist::currentMediaChanged,this,&Dialog::updateSongLyclist);//歌曲变化就查找和提取歌词

    connect(player,&QMediaPlayer::stateChanged,this,&Dialog::updateSongLyclist);//第一首歌播放时候没有更新歌词，笨办法就是改变状态就更新


    //最小化至托盘功能设置
    //新建QSystemTrayIcon对象
    mSysTrayIcon = new QSystemTrayIcon(this);
    //新建托盘要显示的icon
    QIcon ic = QIcon("/Qt1/MyPlayer01/images/logo.png");
    //将icon设到QSystemTrayIcon对象中
    mSysTrayIcon->setIcon(ic);
    //当鼠标移动到托盘上的图标时，会显示此处设置的内容
    mSysTrayIcon->setToolTip(QObject::trUtf8("音乐"));
    //在系统托盘显示此对象
    mSysTrayIcon->show();

    //connect(mSysTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason)));
    connect(mSysTrayIcon, &QSystemTrayIcon::activated, [=](){
       Dialog::on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason::DoubleClick);
    });
    //上面注释掉的connect的slot不存在，原因是前面参数的个数不能小于后面的参数个数，可运用lamda表达式解决此问题


    /*
简单来说，Lambda函数也就是一个函数。

一、它的语法定义如下：
[capture](parameters) mutable ->return-type{statement}

各个参数说明：

1.[capture]：捕捉列表。捕捉列表总是出现在Lambda函数的开始处。实际上，[]是Lambda引出符。编译器根据该引出符判断接下来的代码是否是Lambda函数。捕捉列表能够捕捉上下文中的变量以供Lambda函数使用;

2.(parameters)：参数列表。与普通函数的参数列表一致。如果不需要参数传递，则可以连同括号“()”一起省略;

3.mutable：mutable修饰符。默认情况下，Lambda函数总是一个const函数，mutable可以取消其常量性。在使用该修饰符时，参数列表不可省略（即使参数为空）;

4.->return-type：返回类型。用追踪返回类型形式声明函数的返回类型。我们可以在不需要返回值的时候也可以连同符号”->”一起省略。此外，在返回类型明确的情况下，也可以省略该部分，让编译器对返回类型进行推导;

5.{statement}：函数体。内容与普通函数一样，不过除了可以使用参数之外，还可以使用所有捕获的变量。

二、与普通函数最大的区别是，除了可以使用参数以外，Lambda函数还可以通过捕获列表访问一些上下文中的数据。

具体地，捕捉列表描述了上下文中哪些数据可以被Lambda使用，以及使用方式（以值传递的方式或引用传递的方式）。语法上，在“[]”包括起来的是捕捉列表，捕捉列表由多个捕捉项组成，并以逗号分隔。捕捉列表有以下几种形式：

1.[var]表示值传递方式捕捉变量var；
2.[=]表示值传递方式捕捉所有父作用域的变量（包括this）；
3.[&var]表示引用传递捕捉变量var；
4.[&]表示引用传递方式捕捉所有父作用域的变量（包括this）；
5.[this]表示值传递方式捕捉当前的this指针。

上面提到了一个父作用域，也就是包含Lambda函数的语句块，说通俗点就是包含Lambda的“{}”代码块。上面的捕捉列表还可以进行组合，例如：

1.[=,&a,&b]表示以引用传递的方式捕捉变量a和b，以值传递方式捕捉其它所有变量;
2.[&,a,this]表示以值传递的方式捕捉变量a和this，引用传递方式捕捉其它所有变量。

不过值得注意的是，捕捉列表不允许变量重复传递。下面一些例子就是典型的重复，会导致编译时期的错误。例如：

3.[=,a]这里已经以值传递方式捕捉了所有变量，但是重复捕捉a了，会报错的;
4.[&,&this]这里&已经以引用传递方式捕捉了所有变量，再捕捉this也是一种重复。
     */



    //connect(this,SIGNAL(keyPressEvent(QKeyEvent *event)),this,SLOT(on_play_Btn_clicked()));
//以下功能有点问题暂未解决，先不进行实现
    //connect(ui->full_Btn,SIGNAL(clicked()),this,SLOT(fullscreen()));//设置全屏

    //connect(ui->halffull_Btn,SIGNAL(clicked()),this,SLOT(halfscreen()));	//退出全屏

    connect(ui->playPattern_Btn,SIGNAL(clicked()),this,SLOT(on_playPattern_Btn_clicked()));

    //connect(ui->log_Btn,SIGNAL(clicked()),this,SLOT(show_slot()));

    //connect 有不同的写法，replyFinished也就是replyFinished()。
    network_manager = new QNetworkAccessManager();
    network_request = new QNetworkRequest();
    connect(network_manager, &QNetworkAccessManager::finished, this, &Dialog::replyFinished);

    network_manager2 = new QNetworkAccessManager();//类似于QHttp用来处理网络通讯
    network_request2 = new QNetworkRequest();
    connect(network_manager2, &QNetworkAccessManager::finished, this, &Dialog::replyFinished2);

    //connect(this->ui->pushButton_search,SIGNAL(clicked(bool)),this,SLOT(search(QString str)));
    connect(this->ui->pushButton_search, &QPushButton::clicked, [=](){
       search(this->ui->search_LineEdit->text());
    });

    QTableWidget *tableWidget=new QTableWidget;

    //加载动态图
    movie = new QMovie("/Qt1/MyPlayer01/images/bottom.gif");
    //设置动态图大小
    size.setWidth(1331);
    size.setHeight(131);
    movie->setScaledSize(size);
    //标签加载动态图
    ui->label_bottom->setMovie(movie);
    //动态图开始
    movie->start();

    //加载动态图
    movie1 = new QMovie("/Qt1/MyPlayer01/images/snow.gif");
    //设置动态图大小
    size1.setWidth(861);
    size1.setHeight(571);
    movie1->setScaledSize(size1);
    //标签加载动态图
    ui->label_lrc->setMovie(movie1);
    //动态图开始
    movie1->start();
}


//设置整体界面的背景
void Dialog::paintEvent(QPaintEvent *event)
{
    QPainter myPainter(this);
    myPainter.setOpacity(0.5);          //背景图片透明度
    myPainter.drawPixmap(0,0,this->width(),this->height(),QPixmap("/Qt1/MyPlayer01/images/bac.jpg"));
}

void Dialog::on_play_Btn_clicked()                              //播放按钮
{
    if(!(playList->isEmpty()))
    {
        isPlay = !isPlay;
        if(!isPlay)
        {
            ui->play_Btn->setToolTip(QString("播放"));
            player->pause();
            myLrc_widget->stopLrc();
        }
        else
        {
            ui->play_Btn->setToolTip(QString("暂停"));
            player->setPosition(music_position);
            player->play();
            myLrc_widget->startLrc();
        }
    }
    else
    {
        QMessageBox::warning(this,"提示: ","播放列表为空!\n请先添加歌曲!");
    }
}

void Dialog::on_muteButton_clicked()                            //是否静音
{
    isMute = !isMute;
    if(!isMute)
    {
        ui->volumeControl->setValue(50);
    }
    else
    {
        ui->volumeControl->setValue(0);
    }
}

void Dialog::on_volumeControl_valueChanged(int value)           //调节音量
{
    music_volume = value;
    player->setVolume(music_volume);
}

void Dialog::on_add_Btn_clicked()       //添加歌曲
{
    QString songFileName,songName;
    QFileInfo info;
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("打开文件"),"",tr("music(*.mp3)"));
    if(!fileNames.isEmpty())
    {
        bool add = true;
    }
    for(int i = 0; i < fileNames.size(); i ++)
    {
        playList -> addMedia(QUrl::fromLocalFile(fileNames.at(i)));//将音乐文件逐一加入到播放列表

        songFileName = fileNames.at(i);
        info = QFileInfo(songFileName);
        songName = info.fileName();
        ui->list->addItem(songName);//添加歌曲名到界面的歌曲列表
    }

    if(ui->search_LineEdit->text().isEmpty())
    {
        playList->setCurrentIndex(0);
        player->setPlaylist(playList);
    }

}

void Dialog::on_mediaStatusChanged(QMediaPlayer::MediaStatus status)            //播放完成自动跳转下一曲
{
    if(status == QMediaPlayer::EndOfMedia)
    {
        //播放完成
        on_next_Btn_clicked();
    }
}

void Dialog::on_seekSlider_sliderReleased()     //歌曲进度条改变，播放位置相应改变
{
    music_position = ui->seekSlider->value();
    player->setPosition(music_position);
}

void Dialog::on_playPattern_Btn_clicked()           //播放顺序
{
    playPattern++;
    playPattern = playPattern % 3;
    if(playPattern == 0)
    {
        ui->playPattern_Btn->setStyleSheet("QPushButton#playPattern_Btn{border-image: url(:/images/shunxu.png);}");
        playList->setPlaybackMode(QMediaPlaylist::Sequential);
        ui->playPattern_Btn->setToolTip(QString("顺序播放"));
    }
    else if(playPattern == 1)
    {
        ui->playPattern_Btn->setStyleSheet("QPushButton#playPattern_Btn{border-image: url(:/images/danqu.png);}");
        playList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
        ui->playPattern_Btn->setToolTip(QString("单曲循环"));
    }
    else
    {
        ui->playPattern_Btn->setStyleSheet("QPushButton#playPattern_Btn{border-image: url(:/images/suiji.png);}");
        playList->setPlaybackMode(QMediaPlaylist::Random);
        ui->playPattern_Btn->setToolTip(QString("随机播放"));
    }
}

void Dialog::on_next_Btn_clicked()          //下一曲
{
    player->setMedia(playList->media(playList->nextIndex()));
    playList->setCurrentIndex(playList->nextIndex());
    isPlay = false;
    on_play_Btn_clicked();
    ui->list->setCurrentRow(playList->currentIndex());
    ui->music_name_label->setText(ui->list->currentItem()->text());
    if(playList->currentIndex() >= 0 && playList->currentIndex() < playList->mediaCount())
    {
        get_lrcStrTime();
    }
    if(playList->currentIndex() >= 0 && playList->currentIndex() >= playList->mediaCount()) {
        QMessageBox::warning(this,"警告","这已经是最后一首歌啦！如果还想听的话请添加新的歌曲",QMessageBox::Yes);
    }
}

void Dialog::on_last_Btn_clicked()          //上一曲
{
    player->setMedia(playList->media(playList->previousIndex()));
    playList->setCurrentIndex(playList->previousIndex());
    isPlay = false;
    on_play_Btn_clicked();
    ui->list->setCurrentRow(playList->currentIndex());
    ui->music_name_label->setText(ui->list->currentItem()->text());
    if(playList->currentIndex() >= 0 && playList->currentIndex() < playList->mediaCount())
    {
        get_lrcStrTime();
    }
}


//参数：const QModelIndex &index
void Dialog::on_list_doubleClicked(const QModelIndex &index)        //双击列表，播放
{
    player->setMedia(playList->media(index.row()));
    playList->setCurrentIndex(index.row());
    ui->list->setCurrentRow(index.row());
    isPlay = false;
    on_play_Btn_clicked();
    ui->music_name_label->setText(ui->list->currentItem()->text());
    lrcindex = 0;

    if(playList->currentIndex() >= 0 && playList->currentIndex() < playList->mediaCount())
    {
        get_lrcStrTime();
    }
    qDebug()<<lrcList.count();
}

//关闭界面
void Dialog::on_close_Btn_clicked()
{
    delete myLrc_widget;
    myLrc_widget = NULL;
    this->close();
}

//使界面最小化
void Dialog::on_min_Btn_clicked()
{
    this->hide();
}

//使标题滚动显示
void Dialog::doMoveTitle()
{
    if(titleweizhi < titleStr.length())
    {
        QString temp = titleStr.mid(titleweizhi)+titleStr.mid(0,titleweizhi);
        //mid(int position, int n = -1)方法,第一个参数position指定截取字符串起始位置，第二个参数n指定截取字符串长度。
        ui->title_label->setText(temp);
        titleweizhi++;
    }
    else
    {
        titleweizhi = 0;
    }
}

void Dialog::on_volumeValue_linkActivated()//实时音量数字化显示
{

    int volumeval=ui->volumeControl->value();
    ui->volumeValue->setText(QString::number(volumeval));

}

void Dialog::changeVolume()
{
     player->setVolume(ui->volumeControl->value());//通过滑块值设置音量
    //qDebug()<<"位置"<<ui->VolumeSlider->value();
}

QStringList Dialog::getFileNames(const QString &path)//获取多文件名
{
    QDir dir(path);
    QStringList nameFilters;
    nameFilters<<"*.mp3";
    QStringList files=dir.entryList(nameFilters,QDir::Files|QDir::Readable,QDir::Name); //获取文件信息
    return files;
}

void Dialog::on_LycButton_cliked()//显示歌词
{

    if(Lyric->isHidden())//如果隐藏的话就show，否则就hide
    {
        Lyric->show();
     }
    else
    {
        Lyric->hide();
    }

}

void Dialog::updateSongLrc(qint64 position)//随时间变化而变化显示歌词
{
    Lyric->showcontent(position);//更新歌词位置
}

void Dialog::updateSongLyclist()
{
    int index=playList->currentIndex();//获取当前位置
    QString songname_daihouzui=fileList.at(index);//提取在当前位置的文件名
    QStringList songnamelist = songname_daihouzui.split(".");//QString字符串分割函数
    QString songname=songnamelist[0];
   // qDebug()<<"歌词歌曲名："<<songname;
    QString lycpath=MusicPath+"/" +songname+".lrc";
    Lyric->process(lycpath);
}

void Dialog::get_lrcStrTime()       //得到歌词字符串，保存到list
{
    if(!playList->isEmpty() && !lrcList.isEmpty())
    {
        int index = playList->currentIndex();

        nowLrcListRow = 0;
        lrcTime.clear();
        lrcStr.clear();
        ui->Lrc_list->clear();
        if(index < lrcList.count())
        {
            QString str = lrcList.at(index);
            QStringList li = str.split("[");
            foreach (QString temp, li) {
                if(temp.contains("]"))
                {
                    QString tempstr = temp.split("]").at(0);
                    //                qDebug()<<tempstr;
                    //                lrcTime.append(tempstr);
                    //                lrcTime.append(tempstr.split(".").at(0));
                    QString ms = tempstr.split(".").at(1);
                    ms = QString("%1").arg(ms.toInt(),3,10,QLatin1Char('0'));
                    lrcTime.append(tempstr.split(".").at(0)+":"+ms);
                    if(QString(temp.split("]").at(1)).contains("\n"))
                    {
                        lrcStr.append(QString(temp.split("]").at(1)).remove("\n"));
                    }
                }
            }
            if(lrcStr.count()>10)
            {
                for(int i = 0;i < 10;i++)
                {
                    QListWidgetItem *add_item = new QListWidgetItem(ui->Lrc_list);
                    add_item->setText(lrcStr.at(i));
                    //设置item项中的文字位置
                    add_item->setTextAlignment(Qt::AlignHCenter);
                    add_item->setSizeHint(QSize(ui->Lrc_list->width(),ui->Lrc_list->height()/10));
                    ui->Lrc_list->insertItem(i,add_item);
                }
            }
            else
            {
                for(int i = 0;i < lrcStr.count();i++)
                {
                    QListWidgetItem *add_item = new QListWidgetItem(ui->Lrc_list);
                    add_item->setText(lrcStr.at(i));
                    //设置item项中的文字位置
                    add_item->setTextAlignment(Qt::AlignHCenter);
                    add_item->setSizeHint(QSize(ui->Lrc_list->width(),ui->Lrc_list->height()/10));
                    ui->Lrc_list->insertItem(i,add_item);
                }
                for(int i = lrcStr.count();i < 10;i++)
                {
                    QListWidgetItem *add_item = new QListWidgetItem(ui->Lrc_list);
                    add_item->setText("");
                    //设置item项中的文字位置
                    add_item->setTextAlignment(Qt::AlignHCenter);
                    add_item->setSizeHint(QSize(ui->Lrc_list->width(),ui->Lrc_list->height()/10));
                    ui->Lrc_list->insertItem(i,add_item);
                }
            }
            lrcListFirstAdd = 0;
            nowLrcListRow = 0;
        }
    }
}

void Dialog::on_player_Data_change()
{
    ui->list->setCurrentRow(playList->currentIndex());
    QString music_name = player->metaData(QMediaMetaData::Title).toString();        //歌曲名字
    QString author= player->metaData(QMediaMetaData::Author).toString();            //作者
    /*
    if(music_name == "" && author == "")
    {
        if(!ui->list->count() == 0 )
        {
            //ui->music_name_label->setText(ui->list->currentItem()->text());
        }
        else
            ui->music_name_label->setText("");
    }
    else
    {
        //ui->music_name_label->setText(music_name+"-"+author);
    }
    */

    int h = player->duration()/3600000;
    int m = (player->duration()%3600000)/60000;
    int s = ((player->duration()%3600000)%60000)/1000;
    int ms = ((player->duration()%3600000)%60000)%1000;
    QTime time(h,m,s,ms);     //音乐时长
    ui->time_label->setText(time.toString("hh:mm:ss"));
    ui->now_time_label->setText("00:00:00");

    ui->seekSlider->setMinimum(0);
    ui->seekSlider->setMaximum(player->duration());
    music_position = 0;

    if(playList->currentIndex() >= 0 && playList->currentIndex() < playList->mediaCount())
    {
        get_lrcStrTime();
    }
}

void Dialog::on_player_position_change(qint64 position)                         //播放歌曲进度改变，改变时间显示
{
    ui->seekSlider->setValue(position);
    music_position = position;

    int h = music_position/3600000;
    int m = (music_position%3600000)/60000;
    int s = ((music_position%3600000)%60000)/1000;
    int ms = ((music_position%3600000)%60000)%1000;
    QString timestr;
    QTime time(h,m,s,ms);
    ui->now_time_label->setText(time.toString("hh:mm:ss"));

    QTime time2(h,m,s,ms);
    timestr = time2.toString("mm:ss");

    for(int i = 0;i < lrcTime.count();i++)
    {
        if(time <= QTime::fromString(lrcTime.at(i),"mm:ss:zzz"))
        {
            // i 为下一次唱的歌词
            QString mm1;
            QString ss1;
            QString zzz1;
            QString mm2;
            QString ss2;
            QString zzz2;

            if(lrcindex != i || i == 0)
            {
                if(i == 0)
                {
                    myLrc_widget->setLabel_1_text(lrcStr.at(i));
                    myLrc_widget->setLabel_2_text(lrcStr.at(i + 1));
                }
                else
                {
                    if(nowIsFirst)
                    {
                        myLrc_widget->setLabel_2_text(lrcStr.at(i));
                        nowIsFirst = !nowIsFirst;
                    }
                    else
                    {
                        myLrc_widget->setLabel_1_text(lrcStr.at(i));
                        nowIsFirst = !nowIsFirst;
                    }
                }
                if(lrcindex <= 4)              //前5行   直接下来  歌词不变
                {
                    ui->Lrc_list->item(lrcindex)->setTextColor(Qt::red);
                    for(int j = 0;j < lrcindex;j++)
                    {
                        ui->Lrc_list->item(j)->setTextColor(Qt::black);
                    }
                }
                else            //超过5   通过改变每行显示的歌词来改变进度
                {
                    for(int j = 0;j < 4;j++)
                    {
                        ui->Lrc_list->item(j)->setText(lrcStr.at(lrcindex - 4 + j));
                    }
                    for(int j = 0;j < 6;j++)
                    {
                        if(lrcindex + j < lrcStr.count())
                        {
                            ui->Lrc_list->item(4+j)->setText(lrcStr.at(lrcindex + j));
                        }
                        else
                        {
                            ui->Lrc_list->item(4+j)->setText("");
                        }
                    }
                }
                lrcindex = i;
            }
            if(i != 0)
            {
                mm1 = QString(lrcTime.at(i)).split(":").at(0);
                ss1 = QString(lrcTime.at(i)).split(":").at(1);
                zzz1 = QString(lrcTime.at(i)).split(":").at(2);

                mm2 = QString(lrcTime.at(i - 1)).split(":").at(0);
                ss2 = QString(lrcTime.at(i - 1)).split(":").at(1);
                zzz2 = QString(lrcTime.at(i - 1)).split(":").at(2);
                int y = (mm1.toInt() - mm2.toInt())*60*1000 + (ss1.toInt() - ss2.toInt())*1000 + (zzz1.toInt() - zzz2.toInt());
                int x = (m - mm2.toInt())*60*1000 + (s - ss2.toInt())*1000 + (ms - zzz2.toInt());
                myLrc_widget->setProportion(x,y);
            }
            break;
        }
    }
}

//激活缩小至托盘功能
void Dialog::on_activatedSysTrayIcon(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason){
    case QSystemTrayIcon::Trigger:
        //单击托盘图标
        break;
    case QSystemTrayIcon::DoubleClick:
        //双击托盘图标
        //双击后显示主程序窗口
        this->show();
        showNormal();
        raise();
        activateWindow();
        break;
    default:
        break;
    }
}

void Dialog::keyPressEvent(QKeyEvent *event)            //按空格播放
{
    if(event->key() == Qt::Key_Space)
    {
        on_play_Btn_clicked();
    }
}

//以下功能有点问题，暂不进行实现
/*
void Dialog::fullscreen()
{
    //获取桌面屏幕大小
    QDesktopWidget *system_screen=QApplication::desktop();
    QRect desktop_screen = system_screen->screenGeometry();

    ui->Lrc_list->setWindowFlags(Qt::Dialog);//该函数表示将窗口设为顶层窗口，因为函数showFullScreen只对顶级窗口有效，原始的顶级窗口是this
    setWindowFlags (Qt::Dialog| Qt::FramelessWindowHint);//第一个参数表示此控件是窗口类型，第二个表示去除边框、状态栏等，不加就表示保留这些
    ui->Lrc_list->setFixedSize(desktop_screen.width(),desktop_screen.height());	//将label设置为可用桌面的大小
    ui->Lrc_list->showFullScreen();		//全屏显示函数
}

void Dialog::halfscreen()
{
    ui->Lrc_list->setWindowFlags(Qt::SubWindow);
    //子窗口设置为非顶级窗口，再调用showNormal()还原子窗口，即退出全屏化
    ui->Lrc_list->setFixedSize(861,571);		//将播放窗口label恢复我原始设定的大小
    ui->Lrc_list->showNormal();	//该函数就是退出全屏，还原成原来的状态
}
*/

//该功能较为鸡肋，暂取消对其的实现
//界面跳转函数
/*
void Dialog::show_slot()
{
    this->hide();
    login* log=new login;
    log->show();
}
*/

//点击搜索按钮调用该函数，参数为搜索框内容
void Dialog::search(QString str)
{
   QString KGAPISTR1 = QString("http://mobilecdn.kugou.com/api/v3/search/song?format=json"
                               "&keyword=%1&page=1&pagesize=25").arg(str);
    network_request->setUrl(QUrl(KGAPISTR1));
    network_manager->get(*network_request);
}
//为了简化代码，只使用参数keyword

//槽函数
void Dialog::replyFinished(QNetworkReply *reply)
{
    //获取响应的信息，状态码为200表示正常
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    //无错误返回
    if(reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();  //获取字节
        QString result(bytes);  //转化为字符串
        qDebug()<<result;
        parseJson(result);//该函数用于解析api接口返回的json
    }
    else
    {
        //处理错误
        qDebug()<<"搜索失败";
    }
}

void Dialog::parseJson(QString json)
{
    m_Vectorlist.clear();
    m_ID.clear();
    QString songname_original; //歌曲名
    QString singername;        //歌手
    QString hashStr;           //hash
    QString album_name;        //专辑
    int duration;          	   //时间
    QByteArray byte_array;
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(byte_array.append(json), &json_error);
    if (json_error.error == QJsonParseError::NoError)
    {
        if (parse_doucment.isObject())
        {
            QJsonObject rootObj = parse_doucment.object();
            if (rootObj.contains("data"))
            {
                QJsonValue valuedata = rootObj.value("data");
                if (valuedata.isObject())
                {
                    QJsonObject valuedataObject = valuedata.toObject();
                    if (valuedataObject.contains("info"))
                    {
                        QJsonValue valueArray = valuedataObject.value("info");
                        if (valueArray.isArray())
                        {
                            QJsonArray array = valueArray.toArray();
                            int size = array.size();
                            for (int i = 0; i < size; i++)
                            {
                                QJsonValue value = array.at(i);
                                if (value.isObject())
                                {
                                    QJsonObject object = value.toObject();
                                    if (object.contains("songname_original"))//歌曲名
                                    {
                                        QJsonValue AlbumID_value = object.take("songname_original");
                                        if (AlbumID_value.isString())
                                        {
                                            songname_original = AlbumID_value.toString();
                                        }
                                    }
                                    if (object.contains("singername"))//歌手
                                    {
                                        QJsonValue AlbumID_value = object.take("singername");
                                        if (AlbumID_value.isString())
                                        {
                                            singername = AlbumID_value.toString();
                                        }
                                    }
                                    if (object.contains("album_name"))//专辑
                                    {
                                        QJsonValue AlbumID_value = object.take("album_name");
                                        if (AlbumID_value.isString())
                                        {
                                            album_name = AlbumID_value.toString();
                                        }
                                    }
                                    if (object.contains("hash")) //hash
                                    {
                                        QJsonValue FileHash_value = object.take("hash");
                                        if (FileHash_value.isString())
                                        {
                                            hashStr = FileHash_value.toString();
                                            //用vector保存每首歌曲的hash
                                            m_Vectorlist.append(FileHash_value.toString());
                                        }
                                    }
                                    if (object.contains("album_id"))
                                    {
                                        QJsonValue FileHash_value = object.take("album_id");
                                        if (FileHash_value.isString())
                                        {
                                            //用vector保存每首歌曲的album_id
                                            m_ID.append(FileHash_value.toString());
                                        }
                                    }
                                    if (object.contains("duration"))//时长
                                    {
                                       QJsonValue AlbumID_value = object.take("duration").toInt();
                                       duration = AlbumID_value.toInt();
                                    }
                                    //将解析出的内容放到列表中
                                    ui->tableWidget->setItem(i,0,new QTableWidgetItem(songname_original));
                                    //文字居中
                                    ui->tableWidget->item(i,0)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                                    ui->tableWidget->setItem(i,1,new QTableWidgetItem(singername));
                                    //static_cast<QLineEdit *>(ui->tableWidget->item(i,0)->)->setEnabled(false);
                                    ui->tableWidget->item(i,1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                                    ui->tableWidget->setItem(i,2,new QTableWidgetItem(album_name));
                                    ui->tableWidget->item(i,2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
                                    QString time = QString("%1:%2").arg(duration/60).arg(duration%60);
                                    ui->tableWidget->setItem(i,3,new QTableWidgetItem(time));
                                    ui->tableWidget->item(i,3)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

                                    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
                                    //设置表单不可以被编辑
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
        qDebug() << json_error.errorString();
    }
}

//双击网络播放列表播放歌曲
void Dialog::on_tableWidget_cellDoubleClicked(int row, int column)
{
    //歌曲请求 row 是行号
    QString KGAPISTR1 =QString("http://www.kugou.com/yy/index.php?r=play/getdata"
    "&hash=%1&album_id=%2&_=1497972864535").arg(m_Vectorlist.at(row)).arg(m_ID.at(row));
    network_request2->setUrl(QUrl(KGAPISTR1));
    //手动复制url放到浏览器可以获取json，但是通过代码不行，必须加上下面这句才可以
    network_request2->setRawHeader("Cookie","kg_mid=2333");
    network_request2->setHeader(QNetworkRequest::CookieHeader, 2333);
    network_manager2->get(*network_request2);
    on_play_Btn_clicked();
}

void Dialog::replyFinished2(QNetworkReply *reply)
{
    //获取响应的信息，状态码为200表示正常
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

        //无错误返回
        if(reply->error() == QNetworkReply::NoError)
        {
            QByteArray bytes = reply->readAll();  //获取字节
            QString result(bytes);  //转化为字符串
            parseJsonSongInfo(result);//解析json
        }
        else
        {
            //处理错误
            qDebug()<<"歌曲播放失败";
        }
}

//拆分解析到的信息
void Dialog::parseJsonSongInfo(QString json)
{
    QString audio_name;//歌手-歌名
    QString play_url;//播放地址
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
                               play_urlStr = play_url_value.toString();      //歌曲的url
                               if(play_urlStr!="")
                               {
                                   qDebug()<<play_urlStr;
                                   player->setMedia(QUrl(play_urlStr));
                                   player->setVolume(50);
                                   player->play();
                               }
                           }
                       }
                       if(valuedataObject.contains("audio_name"))
                       {
                           QJsonValue play_name_value = valuedataObject.take("audio_name");
                           if(play_name_value.isString())
                           {
                               QString audio_name = play_name_value.toString();    //歌曲名字
                               if(audio_name!="")
                               {
                                   //显示
                                   qDebug()<<audio_name;
                                   ui->music_name_label->setText(audio_name);
                               }
                           }
                       }
                   }
               else
                   {
                       qDebug()<<"出错";
                   }
               }
           }
       }
}

Dialog::~Dialog()
{
    delete ui;
}

