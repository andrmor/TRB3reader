

CONFIG += cern_root        #enable CERN ROOT for graph drawing

#---CERN ROOT---
cern_root {
    DEFINES += CERN_ROOT

    INCLUDEPATH += $$system(/usr/local/root/bin/root-config --incdir)
    LIBS += $$system(/usr/local/root/bin/root-config --libs) #-lGeom -lGeomPainter -lGeomBuilder -lMathMore -lMinuit2

#    INCLUDEPATH += c:/root/include
#    LIBS += -Lc:/root/lib/ -llibCore -llibCint -llibRIO -llibNet -llibHist -llibGraf -llibGraf3d -llibGpad -llibTree -llibRint -llibPostscript -llibMatrix -llibPhysics -llibRint -llibMathCore -llibGeom -llibGeomPainter -llibGeomBuilder -llibMathMore -llibMinuit2 -llibThread

    SOURCES += ROOT/cernrootmodule.cpp \
               ROOT/GUI/arasterwindow.cpp \
               ROOT/GUI/agraphwindow.cpp

    HEADERS += ROOT/cernrootmodule.h \
               ROOT/GUI/arasterwindow.h \
               ROOT/GUI/agraphwindow.h

    FORMS   += ROOT/GUI/agraphwindow.ui

    INCLUDEPATH += ROOT
    INCLUDEPATH += ROOT/GUI
}
#-----------

#---DABC---
INCLUDEPATH += /home/andr/Soft/DABC/dabc-master/include
LIBS += -L/home/andr/Soft/DABC/dabc-master/lib -lDabcBase -lDabcMbs -lDabcHadaq
#-----------


QT       += core gui
QT += widgets

TARGET = RootBase
TEMPLATE = app


SOURCES += main.cpp \
        GUI/mainwindow.cpp \
    TRB/trb3datareader.cpp \
    TRB/trb3signalextractor.cpp \
    Common/channelmapper.cpp \
    Common/masterconfig.cpp \
    Common/ajsontools.cpp \
    GUI/mainwindowconfig.cpp \
    Common/afiletools.cpp

HEADERS  += GUI/mainwindow.h \    
    TRB/trb3datareader.h \
    TRB/trb3signalextractor.h \
    Common/channelmapper.h \
    Common/masterconfig.h \
    Common/ajsontools.h \
    Common/afiletools.h

FORMS    += GUI/mainwindow.ui

INCLUDEPATH += Common
INCLUDEPATH += GUI
INCLUDEPATH += TRB
