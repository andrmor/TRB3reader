#include "coreinterfaces.h"
#include "ascriptmanager.h"
#include "afiletools.h"

//#include "TRandom2.h"

#include <cmath>

#include <QScriptEngine>
#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>

// ------------------- CORE ----------------------

AInterfaceToCore::AInterfaceToCore(AScriptManager* ScriptManager) :
    ScriptManager(ScriptManager)
{
  Description = "Unit handles general-purpose opeartions: abort script, basic text output and file save/load";

  H["str"] = "Converts double value to string with a given precision";
  H["print"] = "Print the argument (string) on the script output text field";
  H["clearText"] = "Clear the script output text field";
  H["abort"] = "Abort skript execution.\nOptional string argument is a message to be shown on the script output text field";
  H["save"] = "Add string (second argument) to the file with the name given by the first argument.\n"
              "Save is not performed (and false is returned) if the file does not exist\n"
              "It is a very slow method!\n"
              "Use \"<br>\" or \"\\n\" to add a line break.\n"
              "For Windows users: pathes have to use \"/\" character, e.g. c:/tmp/file.txt\n";
  H["saveArray"] = "Appends an array (or array of arrays) with numeric data to the file.\n"
                   "Save is not performed (and false is returned) if the file does not exist.\n"
                   "For Windows users: pathes have to use \"/\" character, e.g. c:/tmp/file.txt\n";
  H["createFile"] = "Create new or clear an existent file.\n"
                    "The file is NOT rewritten if the second argument is true (or absent) and the file already exists\n"
                    "For Windows users: pathes have to use \"/\" character, e.g. c:/tmp/file.txt\n";
  H["isFileExists"] = "Return true if file exists";
  H["loadColumn"] = "Load a column with ascii numeric data from the file.\nSecond argument is the column number starting from 0.";
  H["loadArray"] = "Load an array of numerics (or an array of numeric arrays if columns>1).\nColumns parameter can be from 1 to 3.";
  H["evaluate"] = "Evaluate script during another script evaluation. See example ScriptInsideScript.txt";

  H["SetNewFileFinder"] = "Configurer for GetNewFiles() function. dir is the search directory, fileNamePattern: *.* for all files. Returns the list of all found files.";
  H["GetNewFiles"] = "Get list (array) of names of new files appeared in the directory configured with SetNewFileFinder() after the last use of this function.";

}

AInterfaceToCore::AInterfaceToCore(const AInterfaceToCore& other) :
    AScriptInterface(other)
{
    Finder_FileNames = other.Finder_FileNames;
    Finder_Dir = other.Finder_Dir;
    Finder_NamePattern = other.Finder_NamePattern;

    ScriptManager = 0; //to be set after copy!!!
}

void AInterfaceToCore::SetScriptManager(AScriptManager *ScriptManager)
{
    this->ScriptManager = ScriptManager;
}

void AInterfaceToCore::abort(const QString message) const
{
  //qDebug() << "In-script abort triggered!";
  ScriptManager->AbortEvaluation(message);
}

QVariant AInterfaceToCore::evaluate(const QString script)
{
    QScriptValue val = ScriptManager->EvaluateScriptInScript(script);
    return val.toVariant();
}

void AInterfaceToCore::sleep(int ms)
{
  if (ms == 0) return;
  QTime t;
  t.restart();
  do
  {
      qApp->processEvents();
      QThread::usleep(100);
  }
  while (t.elapsed()<ms);
}

int AInterfaceToCore::elapsedTimeInMilliseconds()
{
    return ScriptManager->getElapsedTime();
}

void AInterfaceToCore::print(const QString text)
{
    //  qDebug() << text << "on ScriptManager:"<<ScriptManager;
    emit ScriptManager->showMessage(text);
}

void AInterfaceToCore::clearText()
{
    emit ScriptManager->clearText();
}

const QString AInterfaceToCore::str(double value, int precision)
{
    return QString::number(value, 'g', precision);
}

const QString AInterfaceToCore::GetTimeStamp() const
{
    return QDateTime::currentDateTime().toString("H:m:s");
}

const QString AInterfaceToCore::GetDateTimeStamp() const
{
    return QDateTime::currentDateTime().toString("d/M/yyyy H:m:s");
}

bool AInterfaceToCore::save(const QString fileName, QString str) const
{
  if (!QFileInfo(fileName).exists())
    {
      //abort("File does not exist: " + fileName);
      qDebug() << "File does not exist: " << fileName;
      return false;
    }

  QFile file(fileName);
  if ( !file.open(QIODevice::Append ) )
  {
      //abort("Cannot open file for appending:" + fileName);
      qDebug() << "Cannot open file for appending:" << fileName;
      return false;
  }

  //qDebug() << str;
  str.replace("<br>", "\n");
  QTextStream outstream(&file);
  outstream << str;
  return true;
}

bool AInterfaceToCore::saveArray(const QString fileName, const QVariant array) const
{
    QString type = array.typeName();
    if (type != "QVariantList")
    {
        qDebug() << "Cannot extract array in saveColumns function";
        //abort("Cannot extract array in saveColumns function");
        return false;
    }

    if (!QFileInfo(fileName).exists())
      {
        //abort("File does not exist: " + fileName);
        qDebug() << "File does not exist: " << fileName;
        return false;
      }

    QFile file(fileName);
    if ( !file.open(QIODevice::Append ) )
    {
        //abort("Cannot open file for appending:" + fileName);
        qDebug() << "Cannot open file for appending:" << fileName;
        return false;
    }

    QTextStream s(&file);

    QVariantList vl = array.toList();
    QJsonArray ar = QJsonArray::fromVariantList(vl);
    //qDebug() << ar.size();

    for (int i=0; i<ar.size(); i++)
    {
        //qDebug() << ar[i];
        if (ar[i].isArray())
        {
            QJsonArray el = ar[i].toArray();
            //qDebug() << "   "<<el;
            for (int j=0; j<el.size(); j++) s << el[j].toDouble(1e10) << " ";
        }
        else
        {
            //qDebug() << "   "<< ar[i].toDouble();
            s << ar[i].toDouble(1e10);
        }
        s << "\n";
    }

    return true;
}

bool AInterfaceToCore::saveObject(const QString FileName, const QVariant Object, bool CanOverride) const
{
    QString type = Object.typeName();
    if (type != "QVariantMap")
    {
        qDebug() << "Not an object - cannt use saveObject function";
        //abort("Cannot extract array in saveColumns function");
        return false;
    }

    if (QFileInfo(FileName).exists() && !CanOverride)
    {
        //abort("File already exists: " + fileName);
        qDebug() << "File already exists: " << FileName << " Skipping!";
        return false;
    }

    QVariantMap mp = Object.toMap();
    QJsonObject json = QJsonObject::fromVariantMap(mp);
    QJsonDocument saveDoc(json);

    QFile saveFile(FileName);
    if (saveFile.open(QIODevice::WriteOnly))
    {
        saveFile.write(saveDoc.toJson());
        saveFile.close();
    }
    else
    {
        qDebug() << "Cannot open file for writing: " << FileName;
        return false;
    }
    return true;
}

const QVariant AInterfaceToCore::loadColumn(const QString fileName, int column) const
{
  if (column<0 || column>2)
    {
      abort ("Supported loadColumn with column # 0, 1 and 2");
      return QVariant();
    }

  if (!QFileInfo(fileName).exists())
  {
    //abort("File does not exist: " + fileName);
    qWarning() << "File does not exist: " << fileName;
    return QVariant();
  }

  QVector<double> v1, v2, v3;
  int res;
  if (column == 0)
     res = LoadDoubleVectorsFromFile(fileName, &v1);
  else if (column == 1)
     res = LoadDoubleVectorsFromFile(fileName, &v1, &v2);
  else if (column == 2)
     res = LoadDoubleVectorsFromFile(fileName, &v1, &v2, &v3);

  if (res != 0)
      {
        abort("Error reading from file: "+fileName);
        return QVariant();
      }
  QList<QVariant> l;
  for (int i=0; i<v1.size(); i++)
    {
      if (column == 0) l.append(v1[i]);
      else if (column == 1) l.append(v2[i]);
      else if (column == 2) l.append(v3[i]);
    }

  return l;
}

const QVariant AInterfaceToCore::loadArray(const QString fileName, int columns) const
{
  if (columns<0 || columns>3)
    {
      abort ("Supported 1, 2 and 3 columns");
      return QVariant();
    }

  if (!QFileInfo(fileName).exists())
  {
    //abort("File does not exist: " + fileName);
    qWarning() << "File does not exist: " << fileName;
    return QVariant();
  }

  QVector<double> v1, v2, v3;
  int res = -1;
  if (columns == 1)
     res = LoadDoubleVectorsFromFile(fileName, &v1);
  else if (columns == 2)
     res = LoadDoubleVectorsFromFile(fileName, &v1, &v2);
  else if (columns == 3)
     res = LoadDoubleVectorsFromFile(fileName, &v1, &v2, &v3);

  if (res != 0)
      {
        abort("Error reading from file: "+fileName);
        return QVariant();
      }

  QList< QVariant > l;
  for (int i=0; i<v1.size(); i++)
    {
      QList<QVariant> ll;
      ll.append(v1[i]);
      if (columns > 1) ll.append(v2[i]);
      if (columns == 3) ll.append(v3[i]);

      QVariant r = ll;
      l << r;
    }
  return l;
}

const QString AInterfaceToCore::loadText(const QString fileName) const
{
  if (!QFileInfo(fileName).exists())
  {
    //abort("File does not exist: " + fileName);
    qWarning() << "File does not exist: " << fileName;
    return "";
  }

  QString str;
  bool bOK = LoadTextFromFile(fileName, str);
  if (!bOK)
    {
      qWarning() << "Error reading file: " << fileName;
      return "";
    }
  return str;
}

const QString AInterfaceToCore::GetWorkDir() const
{
  return ScriptManager->LastOpenDir;
}

const QString AInterfaceToCore::GetScriptDir() const
{
  return ScriptManager->LibScripts;
}

const QString AInterfaceToCore::GetExamplesDir() const
{
    return ScriptManager->ExamplesDir;
}

QVariant AInterfaceToCore::SetNewFileFinder(const QString dir, const QString fileNamePattern)
{
  Finder_Dir = dir;
  Finder_NamePattern = fileNamePattern;

  QDir d(dir);
  QStringList files = d.entryList( QStringList(fileNamePattern), QDir::Files);
  //  qDebug() << files;

  QVariantList res;
  for (auto& n : files)
  {
      Finder_FileNames << n;
      res << n;
  }
  return res;
}

QVariant AInterfaceToCore::GetNewFiles()
{
  QVariantList newFiles;
  QDir d(Finder_Dir);
  QStringList files = d.entryList( QStringList(Finder_NamePattern), QDir::Files);

  for (auto& n : files)
  {
      if (!Finder_FileNames.contains(n)) newFiles << QVariant(n);
      Finder_FileNames << n;
  }
  return newFiles;
}

#include <QProcess>
const QString AInterfaceToCore::StartExternalProcess(QString command, QVariant arguments, bool waitToFinish, int milliseconds)
{
    QStringList arg;
    QString type = arguments.typeName();
    if (type == "QString") arg << arguments.toString();
    else if (type == "int") arg << QString::number(arguments.toInt());
    else if (type == "double") arg << QString::number(arguments.toDouble());
    else if (type == "QVariantList")
    {
        QVariantList vl = arguments.toList();
        QJsonArray ar = QJsonArray::fromVariantList(vl);
        for (int i=0; i<ar.size(); i++) arg << ar.at(i).toString();
    }
    else
    {
        qDebug() << "Format error in argument list";
        return "bad arguments";
    }

    QString str = command + " ";
    for (QString &s : arg) str += s + " ";
    qDebug() << "Executing external command:" << str;

    QProcess *process = new QProcess(this);
    QString errorString;

    if (waitToFinish)
    {
        process->start(command, arg);
        process->waitForFinished(milliseconds);
        errorString = process->errorString();
        delete process;
    }
    else
    {
        QObject::connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
        process->start(command, arg);
    }

    return errorString;
}

bool AInterfaceToCore::createFile(const QString fileName, bool AbortIfExists) const
{
  if (QFileInfo(fileName).exists())
    {
      if (AbortIfExists)
        {
          //abort("File already exists: " + fileName);
          qDebug() << "File already exists: " << fileName;
          return false;
        }
    }

  //create or clear content of the file
  QFile file(fileName);
  if ( !file.open(QIODevice::WriteOnly) )
  {
      //abort("Cannot open file: "+fileName);
      qDebug() << "Cannot open file: " << fileName;
      return false;
  }
  return true;
}

bool AInterfaceToCore::isFileExists(const QString fileName) const
{
    return QFileInfo(fileName).exists();
}

bool AInterfaceToCore::deleteFile(const QString fileName) const
{
    return QFile(fileName).remove();
}

bool AInterfaceToCore::createDir(const QString path) const
{
    QDir dir(path);
    return dir.mkdir(".");
}

const QString AInterfaceToCore::getCurrentDir() const
{
    return QDir::currentPath();
}

bool AInterfaceToCore::setCirrentDir(const QString path) const
{
    return QDir::setCurrent(path);
}
// ------------- End of CORE --------------

// ------------- MATH ------------

AInterfaceToMath::AInterfaceToMath()    //TRandom2* RandGen)
{
  //srand (time(NULL));

  Description = "Custom math module. Uses std library.";

  //this->RandGen = RandGen;

  H["random"] = "Returns a random number between 0 and 1.\nGenerator respects the seed set by SetSeed method of the sim module!";
  H["gauss"] = "Returns a random value sampled from Gaussian distribution with mean and sigma given by the user";
  H["poisson"] = "Returns a random value sampled from Poisson distribution with mean given by the user";
  H["maxwell"] = "Returns a random value sampled from maxwell distribution with Sqrt(kT/M) given by the user";
  H["exponential"] = "Returns a random value sampled from exponential decay with decay time given by the user";
}

/*
void AInterfaceToMath::setRandomGen(TRandom2 *RandGen)
{
  this->RandGen = RandGen;
}
*/

double AInterfaceToMath::abs(double val)
{
  return std::abs(val);
}

double AInterfaceToMath::acos(double val)
{
  return std::acos(val);
}

double AInterfaceToMath::asin(double val)
{
  return std::asin(val);
}

double AInterfaceToMath::atan(double val)
{
  return std::atan(val);
}

double AInterfaceToMath::atan2(double y, double x)
{
  return std::atan2(y, x);
}

double AInterfaceToMath::ceil(double val)
{
  return std::ceil(val);
}

double AInterfaceToMath::cos(double val)
{
  return std::cos(val);
}

double AInterfaceToMath::exp(double val)
{
  return std::exp(val);
}

double AInterfaceToMath::floor(double val)
{
  return std::floor(val);
}

double AInterfaceToMath::log(double val)
{
  return std::log(val);
}

double AInterfaceToMath::max(double val1, double val2)
{
  return std::max(val1, val2);
}

double AInterfaceToMath::min(double val1, double val2)
{
  return std::min(val1, val2);
}

double AInterfaceToMath::pow(double val, double power)
{
  return std::pow(val, power);
}

double AInterfaceToMath::sin(double val)
{
  return std::sin(val);
}

double AInterfaceToMath::sqrt(double val)
{
  return std::sqrt(val);
}

double AInterfaceToMath::tan(double val)
{
    return std::tan(val);
}

double AInterfaceToMath::round(double val)
{
  int f = std::floor(val);
  if (val>0)
    {
      if (val - f < 0.5) return f;
      else return f+1;
    }
  else
    {
      if (val - f < 0.5 ) return f;
      else return f+1;
    }
}

/*
double AInterfaceToMath::random()
{
  //return rand()/(double)RAND_MAX;

  if (!RandGen) return 0;
  return RandGen->Rndm();
}

double AInterfaceToMath::gauss(double mean, double sigma)
{
  if (!RandGen) return 0;
  return RandGen->Gaus(mean, sigma);
}

double AInterfaceToMath::poisson(double mean)
{
  if (!RandGen) return 0;
  return RandGen->Poisson(mean);
}

double AInterfaceToMath::maxwell(double a)
{
  if (!RandGen) return 0;

  double v2 = 0;
  for (int i=0; i<3; i++)
    {
      double v = RandGen->Gaus(0, a);
      v *= v;
      v2 += v;
    }
  return std::sqrt(v2);
}

double AInterfaceToMath::exponential(double tau)
{
    if (!RandGen) return 0;
    return RandGen->Exp(tau);
}
*/

// ------------- End of MATH -------------
