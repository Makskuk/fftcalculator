QT += widgets multimedia

CONFIG += c++11

TARGET = fftcalculator

TEMPLATE = app

SOURCES += main.cpp \
    fftcalculator.cpp \
    filereader.cpp \
    wavfile.cpp \
    worker.cpp \
    audiodevice.cpp \
    basedatareader.cpp \
    unix-signal-wrapper.cpp \
    mainwindow.cpp

HEADERS += \
    fftcalculator.h \
    filereader.h \
    wavfile.h \
    worker.h \
    audiodevice.h \
    basedatareader.h \
    unix-signal-wrapper.h \
    mainwindow.h

win32: {
    # do not compile this files under windows
    SOURCES -= unix-signal-wrapper.cpp
    HEADERS -= unix-signal-wrapper.h
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

fftreal_dir = ../fftreal

#LIBS += -L$${fftreal_dir}
#LIBS += -lfftreal

win32:CONFIG (release, debug|release): LIBS += -L$${fftreal_dir}/release -lfftreal
else:win32:CONFIG (debug, debug|release): LIBS += -L$${fftreal_dir}/debug -lfftreal
else:unix: LIBS += -L$${fftreal_dir} -lfftreal

INCLUDEPATH += $${fftreal_dir}

CONFIG += install_ok  # Do not cargo-cult this!

FORMS += \
    mainwidget.ui
