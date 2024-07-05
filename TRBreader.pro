
#---Operation mode---
#DEFINES += MULTIBOARD  #setup used in Bern
CONFIG += TextToSpeech  #text-to-speech support, requires Qt's multimedia and texttospeech modules
#comment the line above to define setup with a single (large) TRB3 board
#

#---CERN ROOT---
DEFINES += CERN_ROOT  # obsolete

INCLUDEPATH += $$system(root-config --incdir)
LIBS += $$system(root-config --libs) -lSpectrum #-lGeom -lGeomPainter -lGeomBuilder -lMinuit2

SOURCES += ROOT/cernrootmodule.cpp \
           Common/arandomhub.cpp \
           Common/astopwatch.cpp \
           GUI/alineedit.cpp \
           GUI/atexttospeechconfigurator.cpp \
           GUI/guitools.cpp \
           ROOT/GUI/arasterwindow.cpp \
           ROOT/GUI/agraphwindow.cpp \
           Common/tmpobjhubclass.cpp \
           ROOT/apeakfinder.cpp \
           Script/ScriptInterfaces/agraph_si.cpp \
           Script/ScriptInterfaces/agui_si.cpp \
           Script/ScriptInterfaces/ahist_si.cpp \
           Script/ScriptInterfaces/arootstyle_si.cpp \
           Script/acore_si.cpp \
           Script/aguifromscrwin.cpp \
           Script/aguiwindow.cpp \
           Script/ajscriptmanager.cpp \
           Script/ajscriptworker.cpp \
           Script/amath_si.cpp \
           Script/arootgraphrecord.cpp \
           Script/aroothistrecord.cpp \
           Script/arootobjbase.cpp \
           Script/arootobjcollection.cpp \
           Script/ascripthelpentry.cpp \
           Script/ascripthub.cpp \
           Script/ascriptinterface.cpp \
           Script/ascriptmessenger.cpp \
           Script/ascriptobjstore.cpp \
           Script/avirtualscriptmanager.cpp \
           TRB/trb3timingrecord.cpp

HEADERS += ROOT/cernrootmodule.h \
           Common/arandomhub.h \
           Common/astopwatch.h \
           GUI/alineedit.h \
           GUI/atexttospeechconfigurator.h \
           GUI/guitools.h \
           ROOT/GUI/arasterwindow.h \
           ROOT/GUI/agraphwindow.h \
           Common/tmpobjhubclass.h \
           ROOT/apeakfinder.h \
           Script/ScriptInterfaces/agraph_si.h \
           Script/ScriptInterfaces/agui_si.h \
           Script/ScriptInterfaces/ahist_si.h \
           Script/ScriptInterfaces/arootstyle_si.h \
           Script/acore_si.h \
           Script/aguifromscrwin.h \
           Script/aguiwindow.h \
           Script/ajscriptmanager.h \
           Script/ajscriptworker.h \
           Script/amath_si.h \
           Script/arootgraphrecord.h \
           Script/aroothistrecord.h \
           Script/arootobjbase.h \
           Script/arootobjcollection.h \
           Script/ascripthelpentry.h \
           Script/ascripthub.h \
           Script/ascriptinterface.h \
           Script/ascriptmessenger.h \
           Script/ascriptobjstore.h \
           Script/avirtualscriptmanager.h \
           Script/escriptlanguage.h \
           TRB/trb3timingrecord.h

FORMS   += ROOT/GUI/agraphwindow.ui \
    GUI/atexttospeechconfigurator.ui

INCLUDEPATH += ROOT
INCLUDEPATH += ROOT/GUI
#-----------

#---DABC---
#DABCPATH = /home/exnote/dabc
DABCPATH = /home/andr/dabc
INCLUDEPATH += $$DABCPATH/include
INCLUDEPATH += $$DABCPATH/include/hadaq
LIBS += -L$$DABCPATH/lib/ -lDabcBase -lDabcMbs -lDabcHadaq
#-----------

QT += core gui
QT += widgets
QT += websockets

QT += qml   #this is for qjsengine

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
    GUI/mainwindowscript.cpp \
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
    Net/awebsocketsession.h \
    Net/awebsocketsessionserver.h \
    Script/awebserverinterface.h \
    Net/anetworkmodule.h \
    aservermonitorwindow.h \
    TRB/atrbruncontrol.h \
    TRB/atrbrunsettings.h \
    GUI/abufferdelegate.h

FORMS    += GUI/mainwindow.ui \
    GUI/aeditchannelsdialog.ui \
    aservermonitorwindow.ui \
    GUI/abufferdelegate.ui

INCLUDEPATH += Common
INCLUDEPATH += Script
INCLUDEPATH += Script/ScriptInterfaces
INCLUDEPATH += GUI
INCLUDEPATH += TRB
INCLUDEPATH += Net

#---SPEECH---
TextToSpeech {
    DEFINES += TextToSpeechEnabled
    QT += texttospeech

    SOURCES += GUI/atexttospeech.cpp
    HEADERS += GUI/atexttospeech.h
}
#-----------
