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

    m_player=new QMediaPlayer(this);//������ý�岥��������
    setFixedSize(540,700);
    SetBackground(":/Background/Miku_back.jpg");

    QAudioOutput *audioOutput=new QAudioOutput(this);//������Ƶ�������
    m_player->setAudioOutput(audioOutput );
    audioOutput->setVolume(50.0);

    InitButtons();//��ʼ��
    ui->ProgressBar->installEventFilter(this);//��д���µĺ�������������

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitButtons()
{
    InitIcon();
    InitSlot();
    srand(time(NULL));//�Ե�ǰʱ����Ϊ�������������
    mode=LOOP_MODE;//Ĭ�ϲ���ģʽΪLOOP
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
    ui->lab_MusicInfo->setAttribute(Qt::WA_TranslucentBackground);//͸������
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
    QObject::connect(m_player, &QMediaPlayer::positionChanged,//�˺�������ʱ���͵�ǰλ�ò���
                     [&](qint64 pos) {//�������͸���������lambda���������е���SliderSync��������pos����ú���
                        SliderSync(pos);
                         // qDebug() << "Current position:" << pos;
                     });
    QObject::connect(m_player, &QMediaPlayer::durationChanged,
                     [&](qint64 duration) {
                         TimeSync(duration);
                         // qDebug() << "Current Duration:" << duration;
                     });

    connect(ui->btn_close,SIGNAL(clicked(bool)),this,SLOT(close()));
    connect(ui->btn_min,SIGNAL(clicked(bool)),this,SLOT(showMinimized()));//��С��

    connect(ui->btn_Add,SIGNAL(clicked(bool)),this,SLOT(AddMusic()));//�������
    connect(ui->btn_cover,SIGNAL(clicked(bool)),this,SLOT(Coverimmerse()));//�鿴����
}

void MainWindow::Initpause()//����
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

void MainWindow::AutoPlay(QMediaPlayer::PlaybackState newState){//������һ���ᵼ�¶�ε��øú���
         if(newState == QMediaPlayer::StoppedState){
        int count = ui->w_playlist->count();//����
        int curRow = ui->w_playlist->currentRow();//��ǰ��
        ++curRow;//��һ��
        curRow = curRow >= count ? 0 : curRow;//����������
        ui->w_playlist->setCurrentRow(curRow);
        m_player->setSource(ui->w_playlist->currentItem()->data(Qt::UserRole).value<QUrl>());//�����洢����ʱUser��
        m_player->play();
         }
     }

void MainWindow::AddMusic()
{
    QString curPath ="C:/Users/ASUS/Music";//Ĭ�ϴ�MusicĿ¼
    QString dlgTitle = "Choose music";
    QString filter = "��Ƶ�ļ�(*.mp3 *.wav *.wma *.flac);;�����ļ�(*.*)";//���˷�Ŀ������

    QStringList fileList = QFileDialog::getOpenFileNames(this, dlgTitle, curPath, filter);//Dialog��ʽ���ļ�
    if(fileList.isEmpty())
        return;

    foreach (const auto& item, fileList) {
        QFileInfo fileInfo(item);//��ȡ�ļ����ݣ���Ϣ
        QListWidgetItem *aItem = new QListWidgetItem(fileInfo.baseName());//����һ��QListWidgetItem���󣬲��ڲ����б��н�������Ϊ�ļ�����ȥ����׺
        aItem->setIcon(QIcon(":/Icon/miku.svg"));//�����б��и���ǰ��ͼ��
        aItem->setData(Qt::UserRole, QUrl::fromLocalFile(item));
        ui->w_playlist->addItem(aItem);//���벥���б�
        aItem->setData(Qt::ForegroundRole, QColor(Qt::black));//��ɫ����
    }
    //�������û�����ڲ��ţ��Ϳ�ʼ���ŵ�һ���ļ�
    if(m_player->playbackState() != QMediaPlayer::PlayingState){
        ui->w_playlist->setCurrentRow(0);
        QUrl source = ui->w_playlist->currentItem()->data(Qt::UserRole).value<QUrl>();//�����ִ�������ʱUser��
        m_player->setSource(source);
        m_player->play();
    }
}

void MainWindow::Loadlocalmusic(const QString & filename)//����ָ��Ŀ¼�µ�����,����Bug
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
    if(mode==LOOP_MODE){nextmusic = (Currentmusic + 1) % ui->w_playlist->count();}//��������
    else if (mode==RANDOWM_MODE){
        int cnt=0;
        do{
            nextmusic = rand()%ui->w_playlist->count();
            cnt++;
        }while((nextmusic==Currentmusic)&&(cnt<=3));//��ֹ�������ǰ���ֻ�ֻ��һ��ʱ�ظ����е��¿���
    }
    else if (mode==CYCLE_MODE){
        nextmusic = Currentmusic;}
    ui->w_playlist->setCurrentRow(nextmusic);//��һ��
    PlayCurrentMusic();
}

void MainWindow::PlayPrevSlot()//��PlayNextMusicͬ��
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
    ui->w_playlist->setCurrentRow(prevmusic);//��һ��
    PlayCurrentMusic();
}

void MainWindow::SliderSync(qint64 pos)//���벥��λ��
{
    if(ui->ProgressBar->isSliderDown())
        return;
    ui->ProgressBar->setSliderPosition(pos);
    int secs = pos/1000;//pos��λΪ����
    int mins = secs/60;
    secs %= 60;
    positionTime = QString::asprintf("%d:%02d", mins, secs);//��ʱ��ת��Ϊ�ַ���

    ui->lab_CurrentTime->setText(positionTime);
    ui->lab_CurrentTime->setAlignment(Qt::AlignCenter);//���Ķ���
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

void MainWindow::PlaylistSlot()//Playlist����
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
    QPropertyAnimation anime(window,"pos");//λ�ñ任����
    anime.setDuration(100);//����ʱ��
    anime.setStartValue(QPoint(this->width(),ui->btn_close->width()));//���ÿ�ʼλ��
    anime.setEndValue(QPoint(this->width()-ui->w_playlist->width(),ui->btn_close->width()));
    anime.setEasingCurve(QEasingCurve::InOutQuad);//���建��
    anime.start(QAbstractAnimation::DeleteWhenStopped);//�����������Զ�ɾ����ֹ�ڴ�й¶
    QEventLoop loop;
    connect(&anime,SIGNAL(finished()),&loop,SLOT(quit() ) );
    loop.exec();//�ȴ��������

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

void MainWindow::beautifyInfo(){//���ø�����Ϣ
    QString MusicName = ui->w_playlist->currentItem()->text();
    ui->lab_MusicInfo->setText(MusicName);
    //��������ʾ����ʱ����ʡ�Ժ�
    QFontMetrics fontMetrics(ui->lab_MusicInfo->font());
    QString elidedText = fontMetrics.elidedText(MusicName, Qt::ElideRight, 130);//����130���ؾͽض�
    ui->lab_MusicInfo->setText(elidedText);
    ui->lab_MusicInfo->repaint();
}

void MainWindow::Coverimmerse()//�Ŵ����
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

void MainWindow::CoversizeSync()//��������ߴ�
{
    QSize newSize = ui->lab_Cover->size();

    // ��ȡ���صķ���ͼƬ
    QPixmap pixmap = ui->lab_Cover->pixmap(Qt::ReturnByValue);
    if (!pixmap.isNull())
    {
        // ���ݴ��ڴ�С��������ͼƬ��С
        QSize scaledSize = newSize - QSize(30, 30); // �ʵ���ȥһЩ�߾�
        QPixmap scaledPixmap = pixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->lab_Cover->setPixmap(scaledPixmap);
    }
}

void MainWindow::Mute()//����
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

void MainWindow::do_metaDataChanged()//��ȡ������Ϣ
{
    QMediaMetaData metaData = m_player->metaData();
    QVariant metaImg = metaData.value(QMediaMetaData::ThumbnailImage);//����
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
        ui->btn_cover->setFlat(true); // ʹ��ť����͸��
        ui->btn_cover->setStyleSheet("border: none;"); // �Ƴ���ť�ı߿�
        CoversizeSync();
    } else{
        // qDebug() << "Cover pic didn't find";
        // δ�ҵ�����ͼƬʱ��ʹ��Ĭ�Ϸ���
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

void MainWindow::SetBackground(const QString &filename)//���ñ���
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

void MainWindow::mouseReleaseEvent(QMouseEvent *event)//framelessģʽ���޷�������ק����Ҫoverride
{
    if (event->button() == Qt::LeftButton) {
        isDragging = false;
        event->accept();
    }
    QMainWindow::mouseReleaseEvent(event);//���û��ຯ��
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)// ���������¼�
{
    if (event->type() == QEvent::MouseButtonPress && watched == ui->ProgressBar) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);//���¼�ת��ΪQKeyEvent����
        // ���������λ�ö�Ӧ��ֵ
        if (mouseEvent->button() == Qt::LeftButton)	//�ж����
        {   int clickedValue = ui->ProgressBar->minimum() +
                               ((ui->ProgressBar->maximum() - ui->ProgressBar->minimum()) * mouseEvent->pos().x() / ui->ProgressBar->width());
            ui->ProgressBar->setValue(clickedValue);
            m_player->setPosition(clickedValue); // ���ò�����λ��
            return true;
        }
    } else if (event->type() == QEvent::MouseMove && watched == ui->ProgressBar) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->buttons() & Qt::LeftButton) {
            int draggedValue = ui->ProgressBar->minimum() +
                               ((ui->ProgressBar->maximum() - ui->ProgressBar->minimum()) * mouseEvent->pos().x() / ui->ProgressBar->width());
            ui->ProgressBar->setValue(draggedValue); // ���»����ֵ
            m_player->setPosition(draggedValue); // ���ò�����λ��
            return true;
        }
    }

    return QMainWindow::eventFilter(watched, event);
}
