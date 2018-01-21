

CONFIG += cern_root        #enable CERN ROOT for graph drawing
CONFIG += speech_support   #enables text_to_speech script unit - minimum Qt 5.8.0

#---CERN ROOT---
cern_root {
    DEFINES += CERN_ROOT

#    INCLUDEPATH += $$system(/usr/local/root/bin/root-config --incdir)
#    LIBS += $$system(/usr/local/root/bin/root-config --libs) #-lGeom -lGeomPainter -lGeomBuilder -lMathMore -lMinuit2

    INCLUDEPATH += c:/root/include
    LIBS += -Lc:/root/lib/ -llibCore -llibCint -llibRIO -llibNet -llibHist -llibGraf -llibGraf3d -llibGpad -llibTree -llibRint -llibPostscript -llibMatrix -llibPhysics -llibRint -llibMathCore -llibGeom -llibGeomPainter -llibGeomBuilder -llibMathMore -llibMinuit2 -llibThread

    SOURCES += ROOT/cernrootmodule.cpp \
               ROOT/GUI/arasterwindow.cpp \
               ROOT/GUI/agraphwindow.cpp \
               Script/histgraphinterfaces.cpp \
               Common/tmpobjhubclass.cpp

    HEADERS += ROOT/cernrootmodule.h \
               ROOT/GUI/arasterwindow.h \
               ROOT/GUI/agraphwindow.h \
               Script/histgraphinterfaces.h \
               Common/tmpobjhubclass.h

    FORMS   += ROOT/GUI/agraphwindow.ui

    INCLUDEPATH += ROOT
    INCLUDEPATH += ROOT/GUI
}
#-----------

#---DABC---
#DEFINES += DABC
#INCLUDEPATH += /home/andr/Soft/DABC/dabc_debug/include
#LIBS += -L/home/andr/Soft/DABC/dabc_debug/lib/ -lDabcBase -lDabcMbs -lDabcHadaq
#-----------

#---Speech---
speech_support{
    DEFINES += SPEECH
    QT += texttospeech

    HEADERS += Script/ainterfacetospeech.h
    SOURCES += Script/ainterfacetospeech.cpp
}
#------------

QT += core gui
QT += widgets
QT += script
QT += websockets

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
    Common/afiletools.cpp \
    Script/ainterfacetomessagewindow.cpp \
    Script/ascriptmanager.cpp \
    Script/coreinterfaces.cpp \    
    GUI/mainwindowscript.cpp \
    GUI/ascriptwindow.cpp \
    Common/ahighlighters.cpp \
    Common/amessage.cpp \
    Common/completingtexteditclass.cpp \    
    Script/ainterfacetowaveforms.cpp \
    Script/ainterfacetoconfig.cpp \
    Common/adispatcher.cpp \
    GUI/aeditchannelsdialog.cpp \
    Common/adatahub.cpp \
    Script/ainterfacetodata.cpp \
    Script/ainterfacetoextractor.cpp \
    Script/ainterfacetowebsocket.cpp

HEADERS  += GUI/mainwindow.h \    
    TRB/trb3datareader.h \
    TRB/trb3signalextractor.h \
    Common/channelmapper.h \
    Common/masterconfig.h \
    Common/ajsontools.h \
    Common/afiletools.h \
    Script/ainterfacetomessagewindow.h \
    Script/ascriptinterface.h \
    Script/ascriptmanager.h \
    Script/coreinterfaces.h \    
    GUI/ascriptwindow.h \
    Common/ahighlighters.h \
    Common/amessage.h \
    Common/completingtexteditclass.h \    
    Script/ainterfacetowaveforms.h \
    Script/ainterfacetoconfig.h \
    Common/adispatcher.h \
    GUI/aeditchannelsdialog.h \
    Common/adatahub.h \
    Script/ainterfacetodata.h \
    Script/ainterfacetoextractor.h \
    Script/ainterfacetowebsocket.h

FORMS    += GUI/mainwindow.ui \
    GUI/ascriptwindow.ui \
    GUI/aeditchannelsdialog.ui

INCLUDEPATH += Common
INCLUDEPATH += Script
INCLUDEPATH += GUI
INCLUDEPATH += TRB
