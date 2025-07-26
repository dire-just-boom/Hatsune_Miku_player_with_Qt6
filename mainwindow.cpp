#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QIcon"//set icon
#include "QPixmap"//read image
#include "QAudioOutput"//player
#include "QDir"
#include "QFile"
#include "QFileInfo"//Load music
#include "QMessageBox"//Dialog
#include "QFileDialog"
#include "time.h"//Slider
#include "QDebug"
#include "QPropertyAnimation"//Effect
#include "qmediametadata.h"//MetaData

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::FramelessWindowHint);//frameless
    this->setWindowIcon(QIcon(":/Background/Pj_Sekai_app.png"));//programme icon

    m_player=new QMediaPlayer(this);//创建多媒体播放器对象
    setFixedSize(540,700);
    SetBackground(":/Background/Miku_back.jpg");

    QAudioOutput *audioOutput=new QAudioOutput(this);//创建音频输出对象
    m_player->setAudioOutput(audioOutput );
    audioOutput->setVolume(50.0);

    InitButtons();//初始化
    ui->ProgressBar->installEventFilter(this);//覆写后将新的函数赋给进度条

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitButtons()
{
    InitIcon();
    InitSlot();
    srand(time(NULL));//以当前时间作为种子生成随机数
    mode=LOOP_MODE;//默认播放模式为LOOP
    PlaylistShowFlag=false;
    ui->w_playlist->hide();
    CoverShowFlag=false;
    ui->lab_Cover->hide();
}

void MainWindow::SetButtonStyle(QPushButton *button, const QString &filename)
{
    button->setFixedSize(50,50);
    button->setIcon(QIcon(filename));
    button->setIconSize(QSize(button->width(),button->height()));
    button->setStyleSheet(R"(
           QPushButton{background:rgba(0,0,0,0);border:1px solid rgba(0,0,0,0);}
           QPushButton:hover {background-color: #DCDCDC; }
           QPushButton:pressed {background-color: #76EEC6; }
       )");//Feedback
}

void MainWindow::InitIcon()
{
    SetButtonStyle(ui->btn_prev,":/Icon/song_prev.svg");
    SetButtonStyle(ui->btn_next,":/Icon/song_next.svg");
    SetButtonStyle(ui->btn_play_or_pause,":/Icon/Play.svg");
    SetButtonStyle(ui->btn_playlist,":/Icon/playlist.svg");
    SetButtonStyle(ui->btn_loop,":/Icon/play_loop.svg");
    ui->btn_loop->setIconSize(QSize(ui->btn_loop->width()-10,ui->btn_loop->height()-10));
    SetButtonStyle(ui->btn_Add,":/Icon/Load.svg");
    ui->btn_Add->setIconSize(QSize(ui->btn_Add->width(),ui->btn_Add->height()));
    SetButtonStyle(ui->btn_volumn,":/Icon/Volumn.svg");
    ui->btn_volumn->setIconSize(QSize(ui->btn_volumn->width()-10,ui->btn_volumn->height()-10));
    SetButtonStyle(ui->btn_min,":/Icon/Min.svg");
    ui->btn_min->setIconSize(QSize(ui->btn_min->width()-10,ui->btn_min->height()-10));
    SetButtonStyle(ui->btn_close,":/Icon/Close.svg");
    ui->btn_close->setIconSize(QSize(ui->btn_close->width()-10,ui->btn_close->height()-10));
    SetButtonStyle(ui->btn_cover,":/Icon/Cover.svg");
    ui->btn_cover->setIconSize(QSize(ui->btn_close->width()+10,ui->btn_close->height()+10));

    ui->w_Interact->setStyleSheet("QWidget{"
                                  "border:none;"
                                  "border-radius:20px;"
                                  "background-color:rgba(240,240,240,0.3);}");
    ui->w_playlist->setStyleSheet("QListWidget{"
                                  "border:none;"
                                  "border-radius:20px;"
                                  "background-color:rgba(240,240,240,0.3);}");//QSS smooth border&transparent
    ui->lab_MusicInfo->setAttribute(Qt::WA_TranslucentBackground);//透明背景
    ui->lab_CurrentTime->setAttribute(Qt::WA_TranslucentBackground);
    ui->lab_TotalTime->setAttribute(Qt::WA_TranslucentBackground);
}

void MainWindow::InitSlot()
{
    connect(ui->btn_play_or_pause,SIGNAL(clicked(bool)),this,SLOT(Initpause()));
    connect(ui->btn_loop,SIGNAL(clicked(bool)),this,SLOT(SwitchPlayMode()));
    connect(ui->btn_next,SIGNAL(clicked(bool)),this,SLOT(PlayNextSlot()));
    connect(ui->btn_prev,SIGNAL(clicked(bool)),this,SLOT(PlayPrevSlot()));
    connect(ui->btn_playlist,SIGNAL(clicked(bool)),this,SLOT(PlaylistSlot()));
    connect(ui->btn_volumn,SIGNAL(clicked(bool)),this,SLOT(Mute()));

    connect(m_player, &QMediaPlayer::metaDataChanged, this, &MainWindow::do_metaDataChanged);
    QObject::connect(m_player, &QMediaPlayer::positionChanged,//此函数调用时发送当前位置参数
                     [&](qint64 pos) {//参数发送给匿名函数lambda，又在其中调用SliderSync函数并将pos赋予该函数
                        SliderSync(pos);
                         // qDebug() << "Current position:" << pos;
                     });
    QObject::connect(m_player, &QMediaPlayer::durationChanged,
                     [&](qint64 duration) {
                         TimeSync(duration);
                         // qDebug() << "Current Duration:" << duration;
                     });

    connect(ui->btn_close,SIGNAL(clicked(bool)),this,SLOT(close()));
    connect(ui->btn_min,SIGNAL(clicked(bool)),this,SLOT(showMinimized()));//最小化

    connect(ui->btn_Add,SIGNAL(clicked(bool)),this,SLOT(AddMusic()));//添加音乐
    connect(ui->btn_cover,SIGNAL(clicked(bool)),this,SLOT(Coverimmerse()));//查看封面
}

void MainWindow::Initpause()//播放
{
    if(m_player->isPlaying()){
        m_player->pause();
        ui->btn_play_or_pause->setIcon(QIcon(":/Icon/Play.svg"));
    }
    else{
        m_player->play();
        ui->btn_play_or_pause->setIcon(QIcon(":/Icon/Pause.svg"));
    }
}

void MainWindow::SwitchPlayMode()
{
    if(mode==LOOP_MODE){
        mode=RANDOWM_MODE;
        ui->btn_loop->setIcon(QIcon(":/Icon/play_random.svg"));
    }
    else if(mode==RANDOWM_MODE){
        mode=CYCLE_MODE;
        ui->btn_loop->setIcon(QIcon(":/Icon/play_only.svg"));
    }
    else if (mode==CYCLE_MODE){
        mode=LOOP_MODE;
        ui->btn_loop->setIcon(QIcon(":/Icon/play_loop.svg"));
    }
}

void MainWindow::AutoPlay(QMediaPlayer::PlaybackState newState){//主动下一曲会导致多次调用该函数
         if(newState == QMediaPlayer::StoppedState){
        int count = ui->w_playlist->count();//总数
        int curRow = ui->w_playlist->currentRow();//当前行
        ++curRow;//下一曲
        curRow = curRow >= count ? 0 : curRow;//超上限重置
        ui->w_playlist->setCurrentRow(curRow);
        m_player->setSource(ui->w_playlist->currentItem()->data(Qt::UserRole).value<QUrl>());//歌曲存储在临时User中
        m_player->play();
         }
     }

void MainWindow::AddMusic()
{
    QString curPath ="C:/Users/ASUS/Music";//默认打开Music目录
    QString dlgTitle = "Choose music";
    QString filter = "音频文件(*.mp3 *.wav *.wma *.flac);;所有文件(*.*)";//过滤非目标内容

    QStringList fileList = QFileDialog::getOpenFileNames(this, dlgTitle, curPath, filter);//Dialog形式打开文件
    if(fileList.isEmpty())
        return;

    foreach (const auto& item, fileList) {
        QFileInfo fileInfo(item);//获取文件内容，信息
        QListWidgetItem *aItem = new QListWidgetItem(fileInfo.baseName());//创建一个QListWidgetItem对象，并在播放列表中将其命名为文件名，去除后缀
        aItem->setIcon(QIcon(":/Icon/miku.svg"));//播放列表中歌曲前的图标
        aItem->setData(Qt::UserRole, QUrl::fromLocalFile(item));
        ui->w_playlist->addItem(aItem);//加入播放列表
        aItem->setData(Qt::ForegroundRole, QColor(Qt::black));//黑色字体
    }
    //如果现在没有正在播放，就开始播放第一个文件
    if(m_player->playbackState() != QMediaPlayer::PlayingState){
        ui->w_playlist->setCurrentRow(0);
        QUrl source = ui->w_playlist->currentItem()->data(Qt::UserRole).value<QUrl>();//将音乐储存在临时User中
        m_player->setSource(source);
        m_player->play();
    }
}

void MainWindow::Loadlocalmusic(const QString & filename)//加载指定目录下的音乐,中文Bug
{
    QDir dir(filename);
    if (dir.exists()==false){
        QMessageBox::warning(this,"Folder","File doesn't exist");
        return;
    }
    QFileInfoList filelist = dir.entryInfoList(QDir::Files);
    for(auto doc:filelist){
        if(doc.suffix() == "mp3"||"wav"){//Judge music
            ui->w_playlist->addItem(doc.baseName());
        }
        //current music init
        ui->w_playlist->setCurrentRow(0);
    }
}

void MainWindow::PlayCurrentMusic()
{
    // qDebug()<<ui->w_playlist->currentItem()->text();//show ID
    QUrl source = ui->w_playlist->currentItem()->data(Qt::UserRole).value<QUrl>();
    m_player->setSource(source);
    m_player->play();
    ui->btn_play_or_pause->setIcon(QIcon(":/Icon/Pause.svg"));
}

void MainWindow::PlayNextSlot()
{
    if(ui->w_playlist->count()==0){return;}
    int Currentmusic = ui->w_playlist->currentRow();

    int nextmusic = 0;
    if(mode==LOOP_MODE){nextmusic = (Currentmusic + 1) % ui->w_playlist->count();}//超量重置
    else if (mode==RANDOWM_MODE){
        int cnt=0;
        do{
            nextmusic = rand()%ui->w_playlist->count();
            cnt++;
        }while((nextmusic==Currentmusic)&&(cnt<=3));//防止随机到当前音乐或只有一曲时重复运行导致卡死
    }
    else if (mode==CYCLE_MODE){
        nextmusic = Currentmusic;}
    ui->w_playlist->setCurrentRow(nextmusic);//下一曲
    PlayCurrentMusic();
}

void MainWindow::PlayPrevSlot()//与PlayNextMusic同理
{
    if(ui->w_playlist->count()==0){return;}
    int Currentmusic = ui->w_playlist->currentRow();
    int prevmusic = 0;
    if(mode==LOOP_MODE){
        prevmusic = (Currentmusic - 1);
        if(prevmusic<0){prevmusic=ui->w_playlist->count()-1;}
    }
    else if (mode==RANDOWM_MODE){
        int cnt=0;
        do{
            prevmusic = rand()%ui->w_playlist->count();
            cnt++;
        }while((prevmusic==Currentmusic)&&(cnt<=3));
    }
    else if (mode==CYCLE_MODE){
        prevmusic = Currentmusic;
    }
    ui->w_playlist->setCurrentRow(prevmusic);//上一曲
    PlayCurrentMusic();
}

void MainWindow::SliderSync(qint64 pos)//传入播放位置
{
    if(ui->ProgressBar->isSliderDown())
        return;
    ui->ProgressBar->setSliderPosition(pos);
    int secs = pos/1000;//pos单位为毫秒
    int mins = secs/60;
    secs %= 60;
    positionTime = QString::asprintf("%d:%02d", mins, secs);//将时间转化为字符串

    ui->lab_CurrentTime->setText(positionTime);
    ui->lab_CurrentTime->setAlignment(Qt::AlignCenter);//中心对齐
}

void MainWindow::TimeSync(qint64 duration)
{
    ui->ProgressBar->setRange(0,duration);//duration to length
    int secs = duration / 1000;
    int mins = secs/60;
    secs %= 60;

    durationTime = QString::asprintf("%d:%02d", mins, secs);
    ui->lab_TotalTime->setText(durationTime);//set
    ui->lab_TotalTime->setAlignment(Qt::AlignCenter);
}

void MainWindow::PlaylistSlot()//Playlist显隐
{
    if(PlaylistShowFlag==false){
        ui->w_playlist->show();//show first
        PlaylistInEffect(ui->w_playlist);
        PlaylistShowFlag=true;
    }
    else if(PlaylistShowFlag==true){
        PlaylistOutEffect(ui->w_playlist);
        ui->w_playlist->hide();
        PlaylistShowFlag=false;
    }
}

void MainWindow::PlaylistInEffect(QWidget *window)
{
    QPropertyAnimation anime(window,"pos");//位置变换动画
    anime.setDuration(100);//持续时间
    anime.setStartValue(QPoint(this->width(),ui->btn_close->width()));//设置开始位置
    anime.setEndValue(QPoint(this->width()-ui->w_playlist->width(),ui->btn_close->width()));
    anime.setEasingCurve(QEasingCurve::InOutQuad);//整体缓动
    anime.start(QAbstractAnimation::DeleteWhenStopped);//动画结束后自动删除防止内存泄露
    QEventLoop loop;
    connect(&anime,SIGNAL(finished()),&loop,SLOT(quit() ) );
    loop.exec();//等待动画完成

}

void MainWindow::PlaylistOutEffect(QWidget *window)
{
    QPropertyAnimation anime(window,"pos");
    anime.setDuration(150); 
    anime.setStartValue(QPoint(this->width()-ui->w_playlist->width(),ui->btn_close->width()));
    anime.setEndValue(QPoint(this->width(),ui->btn_close->width()));
    anime.setEasingCurve(QEasingCurve::InOutQuad);
    anime.start(QAbstractAnimation::DeleteWhenStopped);

    QEventLoop loop;
    connect(&anime,SIGNAL(finished()),&loop,SLOT(quit() ) );
    loop.exec();
}

void MainWindow::beautifyInfo(){//设置歌曲信息
    QString MusicName = ui->w_playlist->currentItem()->text();
    ui->lab_MusicInfo->setText(MusicName);
    //超过可显示长度时改用省略号
    QFontMetrics fontMetrics(ui->lab_MusicInfo->font());
    QString elidedText = fontMetrics.elidedText(MusicName, Qt::ElideRight, 130);//超过130像素就截断
    ui->lab_MusicInfo->setText(elidedText);
    ui->lab_MusicInfo->repaint();
}

void MainWindow::Coverimmerse()//放大封面
{
    if(CoverShowFlag==false){
        ui->lab_Cover->show();
        CoverShowFlag=true;
    }
    else if(CoverShowFlag==true){
        ui->lab_Cover->hide();
        CoverShowFlag=false;
    }
}

void MainWindow::CoversizeSync()//调整封面尺寸
{
    QSize newSize = ui->lab_Cover->size();

    // 获取加载的封面图片
    QPixmap pixmap = ui->lab_Cover->pixmap(Qt::ReturnByValue);
    if (!pixmap.isNull())
    {
        // 根据窗口大小调整封面图片大小
        QSize scaledSize = newSize - QSize(30, 30); // 适当减去一些边距
        QPixmap scaledPixmap = pixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->lab_Cover->setPixmap(scaledPixmap);
    }
}

void MainWindow::Mute()//静音
{
    bool mute = m_player->audioOutput()->isMuted();
    m_player->audioOutput()->setMuted(!mute);
    if(mute){
        ui->btn_volumn->setIcon(QIcon(":/Icon/Volumn.svg"));
    }
    else{
        ui->btn_volumn->setIcon(QIcon(":/Icon/Mute.svg"));
    }
}

void MainWindow::do_metaDataChanged()//获取歌曲信息
{
    QMediaMetaData metaData = m_player->metaData();
    QVariant metaImg = metaData.value(QMediaMetaData::ThumbnailImage);//封面
    if(metaImg.isValid())
    {
        // qDebug() << "Cover load successfully" << Qt::endl;
        QString MusicName = ui->w_playlist->currentItem()->text();
        QString MusicAbsPath = "qrc:/Music/" + MusicName + ".mp3";
        QImage img = metaImg.value<QImage>();
        QPixmap musicPixmp = QPixmap::fromImage(img);

        ui->lab_Cover->setPixmap(musicPixmp);
        ui->btn_cover->setIcon(musicPixmp);
        ui->btn_cover->setIconSize(QSize(ui->btn_close->width(),ui->btn_close->height()));
        ui->btn_cover->setFlat(true); // 使按钮背景透明
        ui->btn_cover->setStyleSheet("border: none;"); // 移除按钮的边框
        CoversizeSync();
    } else{
        // qDebug() << "Cover pic didn't find";
        // 未找到封面图片时，使用默认封面
        QPixmap defaultCover(":/Icon/Cover.svg");
        ui->btn_cover->setIcon(defaultCover);
        // QPixmap defaultcover(":/Background/Rin.jpg");
        // ui->lab_Cover->setPixmap(defaultcover);
        ui->btn_cover->setIconSize(ui->btn_cover->size());
        ui->btn_cover->setFlat(true);
        ui->btn_cover->setStyleSheet("border: none;");
    }
    beautifyInfo();
}

void MainWindow::SetBackground(const QString &filename)//设置背景
{
    QPixmap pixmap(filename);//create pic
    QSize windowSize = this->size();//obtain size
    QPixmap scalePixmap = pixmap.scaled(windowSize,Qt::IgnoreAspectRatio,
                                        Qt::SmoothTransformation);//adjust pic size
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Window,QBrush(scalePixmap));
    this->setPalette(palette);
}

void MainWindow::mousePressEvent(QMouseEvent *event)//Override(called auto)
{
    if (event->button() == Qt::LeftButton) {
        isDragging = true;
        lastMousePosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - lastMousePosition);
        event->accept();
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)//frameless模式下无法正常拖拽，需要override
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        event->accept();
    }
    QMainWindow::mouseReleaseEvent(event);//调用基类函数
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)// 处理滑块点击事件
{
    if (event->type() == QEvent::MouseButtonPress && watched == ui->ProgressBar) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);//将事件转化为QKeyEvent类型
        // 计算鼠标点击位置对应的值
        if (mouseEvent->button() == Qt::LeftButton)	//判断左键
        {   int clickedValue = ui->ProgressBar->minimum() +
                               ((ui->ProgressBar->maximum() - ui->ProgressBar->minimum()) * mouseEvent->pos().x() / ui->ProgressBar->width());
            ui->ProgressBar->setValue(clickedValue);
            m_player->setPosition(clickedValue); // 设置播放器位置
            return true;
        }
    } else if (event->type() == QEvent::MouseMove && watched == ui->ProgressBar) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->buttons() & Qt::LeftButton) {
            int draggedValue = ui->ProgressBar->minimum() +
                               ((ui->ProgressBar->maximum() - ui->ProgressBar->minimum()) * mouseEvent->pos().x() / ui->ProgressBar->width());
            ui->ProgressBar->setValue(draggedValue); // 更新滑块的值
            m_player->setPosition(draggedValue); // 设置播放器位置
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}
