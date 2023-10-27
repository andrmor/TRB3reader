
#---Operation mode---
#DEFINES += MULTIBOARD  #setup used in Bern
#comment the line above to define setup with a single (large) TRB3 board
#

#---CERN ROOT---
DEFINES += CERN_ROOT

INCLUDEPATH += $$system(root-config --incdir)
LIBS += $$system(root-config --libs) -lSpectrum #-lGeom -lGeomPainter -lGeomBuilder -lMinuit2

SOURCES += ROOT/cernrootmodule.cpp \
           ROOT/GUI/arasterwindow.cpp \
           ROOT/GUI/agraphwindow.cpp \
           Script/histgraphinterfaces.cpp \
           Common/tmpobjhubclass.cpp \
           ROOT/apeakfinder.cpp

HEADERS += ROOT/cernrootmodule.h \
           ROOT/GUI/arasterwindow.h \
           ROOT/GUI/agraphwindow.h \
           Script/histgraphinterfaces.h \
           Common/tmpobjhubclass.h \
           ROOT/apeakfinder.h

FORMS   += ROOT/GUI/agraphwindow.ui

INCLUDEPATH += ROOT
INCLUDEPATH += ROOT/GUI
#-----------

#---DABC---
DEFINES += DABC
#DABCPATH = /home/exnote/dabc
DABCPATH = /home/andr/dabc
INCLUDEPATH += $$DABCPATH/include
INCLUDEPATH += $$DABCPATH/include/hadaq
LIBS += -L$$DABCPATH/lib/ -lDabcBase -lDabcMbs -lDabcHadaq
#-----------

QT += core gui
QT += widgets
QT += script
QT += websockets

TARGET = TRBreader
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
    Script/ainterfacetowebsocket.cpp \
    Common/ahldfileprocessor.cpp \
    Script/ainterfacetohldfileprocessor.cpp \
    Script/ainterfacetomultithread.cpp \
    Net/awebsocketsession.cpp \
    Net/awebsocketsessionserver.cpp \
    Script/awebserverinterface.cpp \
    Net/anetworkmodule.cpp \
    aservermonitorwindow.cpp \
    TRB/atrbruncontrol.cpp \
    TRB/atrbrunsettings.cpp \
    GUI/abufferdelegate.cpp

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
    Script/ainterfacetowebsocket.h \
    Common/ahldfileprocessor.h \
    Script/ainterfacetohldfileprocessor.h \
    Script/ainterfacetomultithread.h \
    Script/ascriptinterfacefactory.h \
    Net/awebsocketsession.h \
    Net/awebsocketsessionserver.h \
    Script/awebserverinterface.h \
    Net/anetworkmodule.h \
    aservermonitorwindow.h \
    TRB/atrbruncontrol.h \
    TRB/atrbrunsettings.h \
    GUI/abufferdelegate.h

FORMS    += GUI/mainwindow.ui \
    GUI/ascriptwindow.ui \
    GUI/aeditchannelsdialog.ui \
    aservermonitorwindow.ui \
    GUI/abufferdelegate.ui

INCLUDEPATH += Common
INCLUDEPATH += Script
INCLUDEPATH += GUI
INCLUDEPATH += TRB
INCLUDEPATH += Net
