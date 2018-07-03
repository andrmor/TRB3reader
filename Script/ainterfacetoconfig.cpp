#include "ainterfacetoconfig.h"
#include "masterconfig.h"
#include "adispatcher.h"
#include "channelmapper.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

AInterfaceToConfig::AInterfaceToConfig(MasterConfig *Config, ADispatcher *Dispatcher) :
  Config(Config), Dispatcher(Dispatcher)
{
    Description = "Gives access to all configuration settings.";
}

QJsonObject* AInterfaceToConfig::MakeConfigJson() const
{
    QJsonObject* js = new QJsonObject;
    Config->WriteToJson(*js);
    return js;
}

QVariant AInterfaceToConfig::getConfigJson()
{
    QJsonObject js;
    Config->WriteToJson(js);
    return js.toVariantMap();
}

void AInterfaceToConfig::setConfigJson(QVariant configJson)
{
    QString type = configJson.typeName();
    if (type != "QVariantMap")
    {
        abort("Failed to set signal values - need object");
        return;
    }

    const QVariantMap mp = configJson.toMap();
    QJsonObject json = QJsonObject::fromVariantMap(mp);

    Dispatcher->LoadConfig(json);
}

void AInterfaceToConfig::saveConfig(const QString FileName)
{
    Dispatcher->SaveConfig(FileName);
}

void AInterfaceToConfig::loadConfig(const QString FileName)
{
    Dispatcher->LoadConfig(FileName);
}

void AInterfaceToConfig::setKeyValue(QString Key, const QVariant val)
{
    LastError = "";
    //qDebug() << Key << val << val.typeName();
    QString type = val.typeName();

    QJsonValue jv;
    QString rep;
    if (type == "int")
      {
        jv = QJsonValue(val.toInt());
        rep = QString::number(val.toInt());
      }
    else if (type == "double")
      {
        jv = QJsonValue(val.toDouble());
        rep = QString::number(val.toDouble());
      }
    else if (type == "bool")
      {
        jv = QJsonValue(val.toBool());
        rep = val.toBool() ? "true" : "false";
      }
    else if (type == "QString")
      {
        jv = QJsonValue(val.toString());
        rep = val.toString();
      }
    else if (type == "QVariantList")
      {
        QVariantList vl = val.toList();
        QJsonArray ar = QJsonArray::fromVariantList(vl);
        jv = ar;
        rep = "-Array-";
      }
      else if (type == "QVariantMap")
        {
          QVariantMap mp = val.toMap();
          QJsonObject ob = QJsonObject::fromVariantMap(mp);
          jv = ob;
          rep = "-Object-";
        }
    else
      {
        abort("Wrong argument type ("+type+") in setKeyValue with the key:\n"+Key);
        return;
      }

    if (!expandKey(Key)) return;  //aborted inside

    QJsonObject JSON;
    Config->WriteToJson(JSON);

    bool ok = modifyJsonValue(JSON, Key, jv);
    if (ok)
      {
        Dispatcher->LoadConfig(JSON);
        return;
      }

    abort("setKeyValue("+Key+", "+rep+") format error:<br>"+LastError);
    return;
}

const QVariant AInterfaceToConfig::getKeyValue(QString Key)
{
    LastError = "";

    if (!expandKey(Key)) return QVariant(); //aborted anyway
    //qDebug() << "Key after expansion:"<<Key;

    QJsonObject obj;
    Config->WriteToJson(obj);
    int indexOfDot;
    QString path = Key;
    do
    {
        indexOfDot = path.indexOf('.');
        QString propertyName = path.left(indexOfDot);
        QString path1 = (indexOfDot>0 ? path.mid(indexOfDot+1) : QString());
        path = path1;
        //qDebug() << "property, path"<<propertyName<<path;

        QString name;
        QVector<int> indexes;
        bool ok = keyToNameAndIndex(propertyName, name, indexes);
        if (!ok)
        {
            abort("getKeyValue for "+Key+" format error");
            return QVariant();
        }
        propertyName = name;
        //qDebug() << "Attempting to extract:"<<propertyName<<indexes;
        QJsonValue subValue = obj[propertyName];
        //qDebug() << "QJsonValue extraction success?" << (subValue != QJsonValue());
        if (subValue == QJsonValue())
          {
            LastError = "Field not found:"+propertyName;
            qDebug() << LastError;
            abort(LastError);
            return QVariant();
          }

        //updating QJsonObject
        if(indexes.isEmpty())
          { //not an array
            if (path.isEmpty())
            {
                //qDebug() << "QJsonValue to attempt to report back:"<<subValue;
                QVariant res = subValue.toVariant();
                //qDebug() << "QVariant:"<<res;
                return res;
            }
            else obj = subValue.toObject();
          }
        else
          { //it is an array
            //if path is not empty, subValue has to be either object or array
                QJsonArray arr = subValue.toArray();
                for (int i=0; i<indexes.size(); i++)
                {
                    int index = indexes.at(i);
                    if (index<0 || index>arr.size()-1)
                      {
                        LastError = "Array index is out of bounds ("+QString::number(index)+") for field name "+propertyName;
                        qDebug() << LastError;
                        abort(LastError);
                        return QVariant();
                      }
                    if (i == indexes.size()-1)
                    {
                        //it is the last index
                        if (path.isEmpty())
                          {
                            QJsonValue jv = arr[index];
                            QVariant res = jv.toVariant();
                            return res;
                          }
                        else obj = arr[index].toObject();
                    }
                    else arr = arr[index].toArray();
                }

                if (obj.isEmpty())
                  {
                    LastError = "Array element of "+propertyName+" is not QJsonObject!";
                    qDebug() << LastError;
                    abort(LastError);
                    return QVariant();
                  }
          }
    }
    while (indexOfDot>0);

    abort("getKeyValue for "+Key+" failed");
    return QVariant();
}

int AInterfaceToConfig::countLogicalChannels() const
{
    return Config->CountLogicalChannels();
}

bool AInterfaceToConfig::isNegativeHardwareChannel(int iHardwChannel) const
{
    return Config->IsNegativeHardwareChannel(iHardwChannel);
}

bool AInterfaceToConfig::isNegativeLogicalChannel(int iLogicalChannel) const
{
    const int iHardwCh = Config->Map->LogicalToHardware(iLogicalChannel);
    return Config->IsNegativeHardwareChannel(iHardwCh);
}

bool AInterfaceToConfig::isIgnoredHardwareChannel(int iHardwChannel) const
{
    return Config->IsIgnoredHardwareChannel(iHardwChannel);
}

bool AInterfaceToConfig::isIgnoredLogicalChannel(int iLogicalChannel) const
{
    const int iHardwCh = Config->Map->LogicalToHardware(iLogicalChannel);
    return Config->IsIgnoredHardwareChannel(iHardwCh);
}

int AInterfaceToConfig::toHardware(int iLogicalChannel) const
{
    int ihardw = Config->Map->LogicalToHardware(iLogicalChannel);
    if ( ihardw < 0 )
    {
        abort("Unmapped logical channel: "+QString::number(iLogicalChannel));
        return -1;
    }
    return ihardw;
}

int AInterfaceToConfig::toLogical(int iHardwChannel) const
{
    int ilogical = Config->Map->HardwareToLogical(iHardwChannel);
    if ( ilogical < 0 )
    {
        abort("Invalid hardware channel: "+QString::number(iHardwChannel));
        return -1;
    }
    return ilogical;
}

bool AInterfaceToConfig::expandKey(QString &Key)
{
    if (Key.startsWith(".") || Key.contains(".."))
      {
        QStringList keys = Key.split(".");
        //qDebug() << keys;
        QStringList res;

        QJsonObject JSON;
        Config->WriteToJson(JSON);

        find(JSON, keys, res);

        if (!LastError.isEmpty())
          {
            abort("Search error for config key "+Key+":<br>" + LastError);
            return false;
          }
        if (res.isEmpty())
          {
            abort("Not found config key "+Key);
            return false;
          }
        if (res.size()>1)
          {
            abort("Search error for config key "+Key+":<br>" +
                  "Match is not unique!");
            return false;
          }

        Key = res.first();
        //qDebug() << "*-----------"<<res;
      }

    return true;
}

void AInterfaceToConfig::find(const QJsonObject &obj, QStringList Keys, QStringList &Found, QString Path)
{
    // script interface replaces ".." or the leading "." with an empty string
    bool fLast = (Keys.size() == 1); //it is the last key in the Keys list
    if (fLast && Keys.last()=="")
      { //bad format - Keys have to end with a concrete key
        LastError = "Bad format - Last object has to be concrete";
        qDebug() << LastError;
        return;
      }

    //qDebug() << "find1 triggered for:"<<Keys;
    QString Key = Keys.first();
    QString Name;
    QVector<int> indexes;
    if (Key != "")
      { //looking for a concrete key
        if (!keyToNameAndIndex(Key, Name, indexes)) return; //format error

        if (obj.contains(Name))
          { //object does contain the key name! (we do not check here if the index is adequate)
            Path += "." + Key;
            if (fLast)
              { //mission accomplished
                Found.append(Path.mid(1)); //remove the starting "." in the found path
                return;
              }
            //pass to the next key in the chain
            Keys.removeFirst();
            if (indexes.isEmpty())
              find(obj[Name].toObject(), Keys, Found, Path);
            else
            {
              QJsonArray ar = obj[Name].toArray();
              for (int i=0; i<indexes.size()-1; i++) ar = ar[indexes[i]].toArray();
              QJsonObject arob = ar[indexes.last()].toObject();
              find(arob, Keys, Found, Path);
            }
          }
        else return; //does not contains the given key
      }
    else
      { // "" Key
        QString Key = Keys.at(1); //this is next key to find
        if (Key == "") return;  //format problem - cannot be "" followed by ""
        //does the object contain the next key?
        if (!keyToNameAndIndex(Key, Name, indexes)) return; //format error
        if  (obj.contains(Name))
          { //object does contain the key!
            //we can reuse the function:
            Keys.removeFirst(); //remove ""
            find(obj, Keys, Found, Path);
          }
        else
          { //have to check every sub-object
            foreach(QString oneKey, obj.keys())
              {
                QJsonValue Val = obj[oneKey];
                if (Val.isObject())
                  find(obj[oneKey].toObject(), Keys, Found, Path+"."+oneKey);
                else if (Val.isArray())
                  {
                    QJsonArray arr = Val.toArray();
                    for (int i=0; i<arr.size(); i++)
                      find(arr[i].toObject(), Keys, Found, Path+"."+oneKey+"["+QString::number(i)+"]");
                  }
                //else do nothing for other types
              }
          }
    }
}

bool AInterfaceToConfig::keyToNameAndIndex(QString Key, QString &Name, QVector<int> &Indexes)
{
    Indexes.clear();
    if (Key.contains('['))
      { // it is an array
        if (Key.startsWith('[') || !Key.endsWith(']'))
          {
            LastError = "Format error: "+Key;
            qDebug() << LastError;
            return false;
          }
        QStringList ArFields = Key.split('[');
        //qDebug() << ArFields;
        Name = ArFields.first();
        ArFields.removeFirst();
        for (int i=0; i<ArFields.size(); i++)
        {
            QString a = ArFields[i];
            if (!a.endsWith(']'))
            {
                LastError = "Format error: "+Key;
                qDebug() << LastError;
                return false;
            }
            a.chop(1);
            bool ok;
            int index = a.toInt(&ok);
            if (!ok)
            {
                LastError = "Format error: "+Key;
                qDebug() << LastError;
                return false;
            }
            Indexes << index;
        }
      }
    else // it is not an array
        Name = Key;

    //qDebug() << "Extracted Name/Indexes:"<<Name<<Indexes;
    return true;
}

bool AInterfaceToConfig::modifyJsonValue(QJsonObject &obj, const QString &path, const QJsonValue &newValue)
{
    int indexOfDot = path.indexOf('.');
    QString propertyName = path.left(indexOfDot);
    QString subPath = indexOfDot>0 ? path.mid(indexOfDot+1) : QString();
    //qDebug() << "subPath:"<<subPath;

    QString name;
    QVector<int> indexes;
    bool ok = keyToNameAndIndex(propertyName, name, indexes);
    if (!ok) return false;
    propertyName = name;

    //qDebug() << "Attempting to extract:"<<propertyName<<indexes;
    QJsonValue subValue = obj[propertyName];
    //qDebug() << "QJsonvalue extraction success?" << (subValue != QJsonValue());
    if (subValue == QJsonValue())
      {
        LastError = "Property not found: "+propertyName;
        qDebug() << LastError;
        return false;
      }

    //updating QJsonObject
    if(indexes.isEmpty())
      { // it is an object
        if (subPath.isEmpty()) subValue = newValue;
        else
        {
            QJsonObject obj1;
            obj1 = subValue.toObject();
            bool ok = modifyJsonValue(obj1, subPath, newValue);
            if (!ok) return false;
            subValue = obj1;
        }
      }
    else
      { // it is an array
        QVector<QJsonArray> arrays;
        arrays << subValue.toArray();
        for (int i=0; i<indexes.size()-1; i++)  //except the last one - it has special treatment
          {
            int index = indexes.at(i);
            if (index<0 || index>arrays.last().size()-1)
              {
                LastError = "Array index is out of bounds for property name "+propertyName;
                qDebug() << LastError;
                return false;
              }
            arrays << arrays.last()[index].toArray();
          }

        if (subPath.isEmpty())
          { //it is the last subkey
             arrays.last()[indexes.last()] = newValue;
             //propagating to parent arrays
             for (int i=arrays.size()-2; i>-1; i--) arrays[i][indexes[i]] = arrays[i+1];
             subValue = arrays.first();
          }
        else
          { //not the last subkey, so it should be an object
             QJsonObject obj1 = arrays.last()[indexes.last()].toObject();
             if (obj1.isEmpty())
               {
                  LastError = "Array element of "+propertyName+" is not QJsonObject!";
                  qDebug() << LastError;
                  return false;
               }
             bool ok = modifyJsonValue(obj1, subPath, newValue);
             if (!ok) return false;
             arrays.last()[indexes.last()] = obj1;
             //propagating to parent arrays
             for (int i=arrays.size()-2; i>-1; i--) arrays[i][indexes[i]] = arrays[i+1];
             subValue = arrays.first();
          }
      }

    obj[propertyName] = subValue;
    return true;
}
