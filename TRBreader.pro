
#---Operation mode---
#CONFIG += TextToSpeech  #text-to-speech support, requires Qt's multimedia and texttospeech modules
#DEFINES += MULTIBOARD  # multiboard setup used in Bern
#

#---CERN ROOT---
DEFINES += CERN_ROOT  # obsolete

INCLUDEPATH += $$system(root-config --incdir)
LIBS += $$system(root-config --libs) -lSpectrum #-lGeom -lGeomPainter -lGeomBuilder -lMinuit2

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

SOURCES +=  main.cpp \
            ROOT/cernrootmodule.cpp \
            ROOT/GUI/arasterwindow.cpp \
            ROOT/GUI/agraphwindow.cpp \
            ROOT/apeakfinder.cpp \
            Common/arandomhub.cpp \
            Common/astopwatch.cpp \
            Common/tmpobjhubclass.cpp \
            Common/channelmapper.cpp \
            Common/masterconfig.cpp \
            Common/ajsontools.cpp \
            Common/afiletools.cpp \
            Common/amessage.cpp \
            Common/completingtexteditclass.cpp \
            Common/adispatcher.cpp \
            Common/ahldfileprocessor.cpp \
            Script/ScriptInterfaces/aconfig_si.cpp \
            Script/ScriptInterfaces/ascriptinterface.cpp \
            Script/ScriptInterfaces/acore_si.cpp \
            Script/ScriptInterfaces/amath_si.cpp \
            Script/ScriptInterfaces/agraph_si.cpp \
            Script/ScriptInterfaces/agui_si.cpp \
            Script/ScriptInterfaces/ahist_si.cpp \
            Script/ScriptInterfaces/amsg_si.cpp \
            Script/ScriptInterfaces/arootstyle_si.cpp \
            Script/ScriptInterfaces/awindowinterfacebase.cpp \
            Script/ScriptInterfaces/ainterfacetowaveforms.cpp \
            Script/ScriptInterfaces/ainterfacetoextractor.cpp \
            Script/ScriptInterfaces/ainterfacetohldfileprocessor.cpp \
            Script/ScriptWindow/ascriptwindow.cpp \
            Script/ScriptWindow/aargumentcounter.cpp \
            Script/ScriptWindow/ahighlighters.cpp \
            Script/ScriptWindow/ascriptbook.cpp \
            Script/ScriptWindow/ascriptexample.cpp \
            Script/ScriptWindow/ascriptexampledatabase.cpp \
            Script/ScriptWindow/ascriptexampleexplorer.cpp \
            Script/ScriptWindow/atabrecord.cpp \
            Script/ScriptWindow/atextedit.cpp \
            Script/ScriptWindow/atextoutputwindow.cpp \
            Script/aguifromscrwin.cpp \
            Script/aguiwindow.cpp \
            Script/ajscriptmanager.cpp \
            Script/ajscriptworker.cpp \
            Script/arootgraphrecord.cpp \
            Script/aroothistrecord.cpp \
            Script/arootobjbase.cpp \
            Script/arootobjcollection.cpp \
            Script/ascripthelpentry.cpp \
            Script/ascripthub.cpp \
            Script/ascriptmessenger.cpp \
            Script/ascriptobjstore.cpp \
            Script/avirtualscriptmanager.cpp \
            TRB/trb3timingrecord.cpp \
            GUI/alineedit.cpp \
            GUI/guitools.cpp \
            GUI/mainwindow.cpp \
            GUI/mainwindowconfig.cpp \
            GUI/aeditchannelsdialog.cpp \
            TRB/trb3datareader.cpp \
            TRB/trb3signalextractor.cpp \
            TRB/atrbruncontrol.cpp \
            TRB/atrbrunsettings.cpp \
            Script/ainterfacetomessagewindow.cpp \
            Script/ainterfacetowebsocket.cpp \
            Script/awebserverinterface.cpp \
            Net/awebsocketsession.cpp \
            Net/awebsocketsessionserver.cpp \
            Net/anetworkmodule.cpp \
            aservermonitorwindow.cpp \
            GUI/abufferdelegate.cpp

HEADERS  += GUI/mainwindow.h \
            ROOT/cernrootmodule.h \
            ROOT/GUI/arasterwindow.h \
            ROOT/GUI/agraphwindow.h \
            ROOT/apeakfinder.h \
            Common/arandomhub.h \
            Common/astopwatch.h \
            Common/tmpobjhubclass.h \
            Common/channelmapper.h \
            Common/masterconfig.h \
            Common/ajsontools.h \
            Common/afiletools.h \
            Common/adispatcher.h \
            Common/amessage.h \
            Common/completingtexteditclass.h \
            Common/ahldfileprocessor.h \
            GUI/alineedit.h \
            GUI/guitools.h \
            Script/ScriptInterfaces/aconfig_si.h \
            Script/ScriptInterfaces/ascriptinterface.h \
            Script/ScriptInterfaces/acore_si.h \
            Script/ScriptInterfaces/amath_si.h \
            Script/ScriptInterfaces/agraph_si.h \
            Script/ScriptInterfaces/agui_si.h \
            Script/ScriptInterfaces/ahist_si.h \
            Script/ScriptInterfaces/amsg_si.h \
            Script/ScriptInterfaces/arootstyle_si.h \
            Script/ScriptInterfaces/awindowinterfacebase.h \
            Script/ScriptInterfaces/ainterfacetowaveforms.h \
            Script/ScriptInterfaces/ainterfacetoextractor.h \
            Script/ScriptInterfaces/ainterfacetohldfileprocessor.h \
            Script/ScriptWindow/ascriptwindow.h \
            Script/ScriptWindow/aargumentcounter.h \
            Script/ScriptWindow/ahighlighters.h \
            Script/ScriptWindow/ascriptbook.h \
            Script/ScriptWindow/ascriptexample.h \
            Script/ScriptWindow/ascriptexampledatabase.h \
            Script/ScriptWindow/ascriptexampleexplorer.h \
            Script/ScriptWindow/atabrecord.h \
            Script/ScriptWindow/atextedit.h \
            Script/ScriptWindow/atextoutputwindow.h \
            Script/aguifromscrwin.h \
            Script/aguiwindow.h \
            Script/ajscriptmanager.h \
            Script/ajscriptworker.h \
            Script/arootgraphrecord.h \
            Script/aroothistrecord.h \
            Script/arootobjbase.h \
            Script/arootobjcollection.h \
            Script/ascripthelpentry.h \
            Script/ascripthub.h \
            Script/ascriptmessenger.h \
            Script/ascriptobjstore.h \
            Script/avirtualscriptmanager.h \
            Script/escriptlanguage.h \
            TRB/trb3timingrecord.h \
            TRB/trb3datareader.h \
            TRB/trb3signalextractor.h \
            TRB/atrbruncontrol.h \
            TRB/atrbrunsettings.h \
            Script/ainterfacetomessagewindow.h \
            Script/ainterfacetowebsocket.h \
            Script/awebserverinterface.h \
            Net/awebsocketsession.h \
            Net/awebsocketsessionserver.h \
            Net/anetworkmodule.h \
            aservermonitorwindow.h \
            GUI/aeditchannelsdialog.h \
            GUI/abufferdelegate.h

FORMS    += GUI/mainwindow.ui \
            GUI/aeditchannelsdialog.ui \
            GUI/abufferdelegate.ui \
            ROOT/GUI/agraphwindow.ui \
            Script/ScriptWindow/ascriptexampleexplorer.ui \
            Script/ScriptWindow/ascriptwindow.ui \
            aservermonitorwindow.ui

INCLUDEPATH += Common
INCLUDEPATH += Script
INCLUDEPATH += Script/ScriptInterfaces
INCLUDEPATH += Script/ScriptWindow
INCLUDEPATH += GUI
INCLUDEPATH += TRB
INCLUDEPATH += Net

#---SPEECH---
TextToSpeech {
    DEFINES += TextToSpeechEnabled
    QT += texttospeech

    SOURCES += GUI/atexttospeech.cpp \
               GUI/atexttospeechconfigurator.cpp
    HEADERS += GUI/atexttospeech.h \
               GUI/atexttospeechconfigurator.h
    FORMS   += GUI/atexttospeechconfigurator.ui
}
#-----------
