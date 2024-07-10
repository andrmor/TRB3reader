#ifndef ASCRIPTHUB_H
#define ASCRIPTHUB_H

#include "escriptlanguage.h"

#include <QObject>
#include <QString>
#include <vector>

class AJScriptManager;
class AScriptInterface;
class AGuiFromScrWin;
class TObject;

#ifdef ANTS3_PYTHON
    class APythonScriptManager;
#endif

class Trb3dataReader;
class ADataHub;
class Trb3signalExtractor;
class AHldFileProcessor;

class AScriptHub : public QObject
{
    Q_OBJECT

public:
    static AScriptHub      & getInstance();
    static AJScriptManager & manager(); // !!!*** to kill

    static void              abort(const QString & message, EScriptLanguage lang);
    static bool              isAborted(EScriptLanguage lang);

    AJScriptManager        & getJScriptManager() {return *JavaScriptM;}
#ifdef ANTS3_PYTHON
    APythonScriptManager   & getPythonManager()  {return *PythonM;}
#endif

    void registerReaderModule(Trb3dataReader * reader) {Reader = reader;}
    void registerExtractorModule(Trb3signalExtractor * extractor) {Extractor = extractor;}
    void registerDataModule(ADataHub * dataHub) {DataHub = dataHub;}
    void registerHldProcessorModule(AHldFileProcessor * hldProcessor) {HldProcessor = hldProcessor;}

    void createInterfaces();

    void addGuiScriptUnit(AGuiFromScrWin * win);
    void finalizeInit(); // run when initialization is finished (all additional script units already registered)

    void outputText(const QString & text, EScriptLanguage lang);
    void outputHtml(const QString & text, EScriptLanguage lang);
    void outputFromBuffer(const std::vector<std::pair<bool,QString>> & buffer, EScriptLanguage lang);
    void clearOutput(EScriptLanguage lang);

    void reportProgress(int percents, EScriptLanguage lang);

    QString getPythonVersion();

private:
    AScriptHub();
    ~AScriptHub();

    AScriptHub(const AScriptHub&)            = delete;
    AScriptHub(AScriptHub&&)                 = delete;
    AScriptHub& operator=(const AScriptHub&) = delete;
    AScriptHub& operator=(AScriptHub&&)      = delete;

    void addCommonInterface(AScriptInterface * interface, QString name);

signals:
    //for gui
    void outputText_JS(QString);
    void outputText_P(QString);
    void outputHtml_JS(QString);
    void outputHtml_P(QString);
    void outputFromBuffer_JS(std::vector<std::pair<bool,QString>> Buffer);
    void outputFromBuffer_P(std::vector<std::pair<bool,QString>> Buffer);
    void showAbortMessage_JS(QString message); // !!!*** remove, use outputHtml
    void showAbortMessage_P(QString message);
    void clearOutput_JS();
    void clearOutput_P();
    void requestUpdateGui();
    void reportProgress_JS(int percent);
    void reportProgress_P(int percent);

    void requestDraw(TObject * obj, QString options, bool fFocus);

private:
    AJScriptManager      * JavaScriptM = nullptr;
#ifdef ANTS3_PYTHON
    APythonScriptManager * PythonM = nullptr;
#endif

public:
    Trb3dataReader      * Reader       = nullptr;
    Trb3signalExtractor * Extractor    = nullptr;
    ADataHub            * DataHub      = nullptr;
    AHldFileProcessor   * HldProcessor = nullptr;

};

#endif // ASCRIPTHUB_H
