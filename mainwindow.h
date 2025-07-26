#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QPushButton>
#include <QMediaPlayer>
#include <QMouseEvent>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

enum PLAYMODE{
    LOOP_MODE,
    RANDOWM_MODE,
    CYCLE_MODE

};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool eventFilter(QObject *watched, QEvent *event);

public slots:
    void InitSlot();
    void Initpause();
    void SwitchPlayMode();
    void PlayNextSlot();
    void PlayPrevSlot();
    void PlaylistSlot();
    void SliderSync(qint64 pos);
    void TimeSync(qint64 durtion);
    void PlayCurrentMusic();
    void Mute();
    void AutoPlay(QMediaPlayer::PlaybackState newState);
    void AddMusic();
    void Coverimmerse();
protected:

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
private:
    Ui::Widget *ui;
    void SetButtonStyle(QPushButton *button, const QString & filename);
    void InitButtons();
    void InitIcon();
    void SetBackground(const QString & filename);

    QMediaPlayer *m_player;
    QAudioOutput *audioOutput = nullptr;

    QPoint mousePoint;
    bool isDragging;
    QPoint lastMousePosition;

    void Loadlocalmusic(const QString & filename);
    PLAYMODE mode;
    QString music_source;


    bool PlaylistShowFlag;
    bool CoverShowFlag;
    void PlaylistInEffect(QWidget *window);
    void PlaylistOutEffect(QWidget *window);

    QString durationTime;
    QString positionTime;
    void do_metaDataChanged();
    void beautifyInfo();

    void CoversizeSync();

};
#endif // MAINWINDOW_H
