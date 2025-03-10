#include "ascripthub.h"
#include "ajscriptmanager.h"
#include "ajsontools.h"
#include "adispatcher.h"

#ifdef ANTS3_PYTHON
    #include "apythonscriptmanager.h"
    #include "aminipython_si.h"
#endif

// SI
#include "acore_si.h"
#include "amath_si.h"
#include "agraph_si.h"
#include "ahist_si.h"
#include "arootstyle_si.h"

AScriptHub & AScriptHub::getInstance()
{
    static AScriptHub instance;
    return instance;
}

#include <QTimer>
void AScriptHub::abort(const QString & message, EScriptLanguage lang)
{
    AScriptHub & hub = getInstance();
#ifdef ANTS3_PYTHON
    if (lang == EScriptLanguage::Python)     hub.PythonM->abort();
#endif
    if (lang == EScriptLanguage::JavaScript) hub.JavaScriptM->abort();

#ifdef ANTS3_PYTHON
    //if (lang == EScriptLanguage::Python)     emit hub.showAbortMessage_P(message);
    if (lang == EScriptLanguage::Python)     QTimer::singleShot(2, [message](){ emit AScriptHub::getInstance().showAbortMessage_P(message); } );
#endif
    //if (lang == EScriptLanguage::JavaScript) emit hub.showAbortMessage_JS(message);
    if (lang == EScriptLanguage::JavaScript) QTimer::singleShot(2, [message](){ emit AScriptHub::getInstance().showAbortMessage_JS(message); } );
}

bool AScriptHub::isAborted(EScriptLanguage lang)
{
    AScriptHub & hub = getInstance();

#ifdef ANTS3_PYTHON
    if (lang == EScriptLanguage::Python)     return hub.PythonM->isAborted();
#endif
    if (lang == EScriptLanguage::JavaScript) return hub.JavaScriptM->isAborted();

    return false;
}

void AScriptHub::addCommonInterface(AScriptInterface * interface, QString name)
{
    JavaScriptM->registerInterface(interface, name);

#ifdef ANTS3_PYTHON
    AScriptInterface * twin = interface->cloneBase();
    PythonM->registerInterface(twin, name);

    if (geoWin) geoWinInterfaces.push_back(dynamic_cast<AGeoWin_SI*>(twin));
#endif
}

#include "agui_si.h"
#include "aguifromscrwin.h"
void AScriptHub::addGuiScriptUnit(AGuiFromScrWin * win)
{
    JavaScriptM->registerInterface(new AGui_JS_SI(win), "gui");
#ifdef ANTS3_PYTHON
    PythonM->registerInterface(new AGui_Py_SI(win), "gui");
#endif
}

void AScriptHub::finalizeInit()
{
#ifdef ANTS3_PYTHON
    PythonM->finalizeInit();
#endif
}

void AScriptHub::outputText(const QString & text, EScriptLanguage lang)
{
    if (lang == EScriptLanguage::JavaScript) emit outputText_JS(text);
    else                                     emit outputText_P(text);
}

void AScriptHub::outputHtml(const QString &text, EScriptLanguage lang)
{
    if (lang == EScriptLanguage::JavaScript) emit outputHtml_JS(text);
    else                                     emit outputHtml_P(text);
}

void AScriptHub::outputFromBuffer(const std::vector<std::pair<bool, QString>> & buffer, EScriptLanguage lang)
{
    if (lang == EScriptLanguage::JavaScript) emit outputFromBuffer_JS(buffer);
    else                                     emit outputFromBuffer_P(buffer);
}

void AScriptHub::clearOutput(EScriptLanguage lang)
{
    if (lang == EScriptLanguage::JavaScript) emit clearOutput_JS();
    else                                     emit clearOutput_P();
}

QString AScriptHub::loadConfig(const QString & fileName)
{
    QJsonObject json;
    bool ok = LoadJsonFromFile(json, fileName);
    if (!ok) return "Failed to open file to read config: " + fileName;

    return loadConfig(json);
}

QString AScriptHub::loadConfig(QJsonObject & json)
{
    // !!!*** add error control?
    Dispatcher->LoadConfig(json, false);
    JSON = json;
    return "";
}

QString AScriptHub::saveConfig(const QString & fileName)
{
    bool ok = SaveJsonToFile(JSON, fileName);
    if (!ok) return "Filed to save config to file: " + fileName;
    return "";
}

#include "masterconfig.h"
void AScriptHub::updateJSON()
{
    MasterConfig::getInstance().WriteToJson(JSON);
}

void AScriptHub::reportProgress(int percents, EScriptLanguage lang)
{
    if (lang == EScriptLanguage::JavaScript) emit reportProgress_JS(percents);
    else                                     emit reportProgress_P(percents);
}

QString AScriptHub::getPythonVersion()
{
#ifdef ANTS3_PYTHON
    return getPythonManager().getVersion();
#else
    return "Not available";
#endif
}

AScriptHub::AScriptHub()
{
    //qDebug() << ">Creating AJScriptManager and Generating/registering script units";
    JavaScriptM = new AJScriptManager();
#ifdef ANTS3_PYTHON
    PythonM = new APythonScriptManager();
#endif
}

#include "ainterfacetowaveforms.h"
#include "ainterfacetoextractor.h"
#include "ainterfacetohldfileprocessor.h"
#include "aconfig_si.h"
void AScriptHub::createInterfaces()
{
    addCommonInterface(new ACore_SI(),         "core");

    //addCommonInterface(new AMath_SI(),         "math");  // conflicts with inbuild Python module "math"
    JavaScriptM->registerInterface(new AMath_SI(), "math");
#ifdef ANTS3_PYTHON
    PythonM->registerInterface(new AMath_SI(),     "Math");
#endif

    addCommonInterface(new AInterfaceToWaveforms(),        "reader");
    addCommonInterface(new AInterfaceToExtractor(),        "extractor");
    addCommonInterface(new AInterfaceToHldFileProcessor(), "hld");

    addCommonInterface(new AConfig_SI(),       "config");
    addCommonInterface(new AGraph_SI(),        "graph");
    addCommonInterface(new AHist_SI(),         "hist");
    //addCommonInterface(new ATree_SI(),         "tree");
    addCommonInterface(new ARootStyle_SI(),    "root");

//JavaScriptM->registerInterface(new AMiniJS_SI(), "mini");  // !!!*** need here?
#ifdef ANTS3_PYTHON
    //PythonM->registerInterface(new AMiniPython_SI(), "mini");
#endif
}

AScriptHub::~AScriptHub()
{
    qDebug() << "Destr for ScriptHub";
#ifdef ANTS3_PYTHON
    delete PythonM; PythonM = nullptr;
#endif
    delete JavaScriptM; JavaScriptM = nullptr;
}
