#-------------------------------------------------
#
# Project created by QtCreator 2017-01-13T18:32:41
#
#-------------------------------------------------

QT       += core gui
QT       += sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtSmokescreen
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    addthreaddialog.cpp \
    sthread.cpp \
    dbmanager.cpp \
    randomnumber.cpp \
    httphandler.cpp

HEADERS  += mainwindow.h \
    addthreaddialog.h \
    sthread.h \
    dbmanager.h \
    randomnumber.h \
    httphandler.h

FORMS    += mainwindow.ui \
    addthreaddialog.ui

RESOURCES += \
    mainwindow.qrc

unix:contains(QMAKE_HOST.arch, x86_64):{
    message("Host is Linux 64bit")
    #LIBS += -L"/usr/lib/x86_64-linux-gnu/" -lvsqlitepp -lsqlite3 -lpcre -lpcrecpp
    LIBS += -L"/usr/lib/x86_64-linux-gnu/" -lsqlite3 -lmagic -lhtmlcxx -lz
}

win32:contains(QMAKE_HOST.arch, x86):{
    #message("Host is Windows 32bit")
    QMAKE_LFLAGS += -static-libgcc -static-libstdc++
    INCLUDEPATH += "C:\Qt\htmlcxx-0.86-compiled_mingw32_static\include" "C:\Qt\sqlite-3160200_compiled_mingw32_static\include" "C:\Qt\libmagic-5.25_compiled_mingw32_static\include"
    #LIBS += -L"/usr/lib/x86_64-linux-gnu/" -lvsqlitepp -lsqlite3 -lpcre -lpcrecpp -lmagic -lregex -lshlwapi
    LIBS += -L"C:\Qt\htmlcxx-0.86-compiled_mingw32_static\lib" -L"C:\Qt\sqlite-3160200_compiled_mingw32_static\lib" -L"C:\Qt\libmagic-5.25_compiled_mingw32_static\lib" -lhtmlcxx -lsqlite3 -lmagic -lregex -lshlwapi -lz
}

win32-g++:{
    message("Target is win32-g++")
    QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++
    LIBS += -lhtmlcxx -lsqlite3 -lmagic -lregex -lshlwapi -lz
}
