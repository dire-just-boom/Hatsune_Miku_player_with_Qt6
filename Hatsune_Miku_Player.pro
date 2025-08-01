QT       += core gui
QT+=svg
QT+=multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    Icon/Addition.svg \
    Icon/Mute.svg \
    Icon/Pause.svg \
    Icon/Play.svg \
    Icon/Volumn.svg \
    Icon/local_music.svg \
    Icon/play_loop.svg \
    Icon/play_only.svg \
    Icon/play_random.svg \
    Icon/playlist.svg \
    Icon/seach.svg \
    Icon/song_next.svg \
    Icon/song_prev.svg \
    Music/sasakure.UK.mp3
