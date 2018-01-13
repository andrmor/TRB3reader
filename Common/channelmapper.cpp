#include "channelmapper.h"

#include <QDebug>

ChannelMapper::ChannelMapper() {}

int ChannelMapper::HardwareToLogical(int iHardwareChannel) const
{
    if ( iHardwareChannel>=0 && iHardwareChannel<ToLogical.size() ) return ToLogical.at(iHardwareChannel);

    //qDebug() << "--- Invalid hardware channel!";
    return -1;
}

int ChannelMapper::LogicalToHardware(int iLogicalChannel) const
{
    if ( iLogicalChannel>=0 && iLogicalChannel<ToLogical.size() ) return ToHardware.at(iLogicalChannel);

    //qDebug() << "--- Invalid logical channel";
    return -1;
}

int ChannelMapper::HardwareToLogicalFast(int iHardwareChannel) const
{
    return ToLogical.at(iHardwareChannel);
}

int ChannelMapper::LogicalToHardwareFast(int iLogicalChannel) const
{
    return ToHardware.at(iLogicalChannel);
}

const QStringList ChannelMapper::PrintToLogical() const
{
    QStringList list;
    for (int ihw=0; ihw<ToLogical.size(); ihw++)
    {
        QString s = QString::number(ihw);

        int ilog = ToLogical.at(ihw);
        if (ilog == -1) s += " is not mapped";
        else s += " -> "+QString::number(ilog);

        list << s;
    }
    return list;
}

const QStringList ChannelMapper::PrintToHardware() const
{
    QStringList list;
    for (int ilog=0; ilog<ToHardware.size(); ilog++)
    {
        QString s = QString::number(ilog);

        int ihw = ToHardware.at(ilog);
        if (ihw == -1) s += " is not mapped";
        else s += " -> "+QString::number(ihw);

        list << s;
    }
    return list;
}

/*
void ChannelMapper::SetChannels_OrderedByHardware(QVector<int> ToLogicalChannelMap)
{
    Clear();
    if (ToLogicalChannelMap.isEmpty()) return;

    int imax = -1;
    for (int ich : ToLogicalChannelMap) if (ich > imax) imax = ich;
    ToLogical = QVector<int>(imax+1, -1);

    for (int iHardware=0; iHardware<ToLogicalChannelMap.size(); iHardware++)
    {
        const int iLogical = ToLogicalChannelMap.at(iHardware);
        ToLogical[iHardware] = iLogical;
    }

    update_ToHardware();
}
*/

void ChannelMapper::SetChannels_OrderedByLogical(QVector<int> ToHardwareChannelMap)
{
    Clear();

    ToHardware = ToHardwareChannelMap;

    update_ToLogical();
}

bool ChannelMapper::UpdateNumberOfHardwareChannels(int NewNumberOfHardwChannels)
{
    if ( NewNumberOfHardwChannels < CountLogicalChannels() )
    {
        qDebug() << "Channel map update error: New number of hardware channels is less than the defined number of logical channels";
        return false;
    }

    if (ToLogical.size() == NewNumberOfHardwChannels) return true; //nothing changed

    update_ToLogical();
    return true;
}

void ChannelMapper::Clear()
{
    ToLogical.clear();
    ToHardware.clear();
}

/*
void ChannelMapper::update_ToHardware()
{
    ToHardware.clear();

    int imax = -1;
    for (int ich : ToLogical) if (ich > imax) imax = ich;
    ToHardware = QVector<int>(imax+1, -1);

    for (int iHardware=0; iHardware<ToLogical.size(); iHardware++)
    {
        const int iLogical = ToLogical.at(iHardware);
        if ( iLogical < 0 ) continue;
        ToHardware[ iLogical ] = iHardware;
    }
}
*/

void ChannelMapper::update_ToLogical()
{
    ToLogical.clear();

    int imax = -1;
    for (int ich : ToHardware) if (ich > imax) imax = ich;
    ToLogical = QVector<int>(imax+1, -1);

    for (int iLogical=0; iLogical<ToHardware.size(); iLogical++)
    {
        int iHardware = ToHardware.at(iLogical);
        if ( iHardware < 0 ) continue;
        ToLogical[ iHardware ] = iLogical;
    }
}

const QString ChannelMapper::Validate() const
{
    if (ToHardware.isEmpty() || ToLogical.isEmpty()) return "Channel map is not defined!";

    //check uniquness
    QSet<int> setHardwToLogical;
    int numUndefined = 0;
    for (int i : ToLogical) if (i == -1) numUndefined++; else setHardwToLogical << i;
    if (setHardwToLogical.size() != ToLogical.size()-numUndefined) return "Channel map is corrupted: degeneracy in ToLogical";
    QSet<int> setLogicalToHardware;
    numUndefined = 0;
    for (int i : ToHardware) if (i == -1) numUndefined++; else setLogicalToHardware << i;
    if (setLogicalToHardware.size() != ToHardware.size()-numUndefined) return "Channel map is corrupted: degeneracy in ToHardware";

    //checking logical->hardware
    int imax = -1;
    for (int ich : ToHardware) if ( ich > imax ) imax = ich;
    QVector<bool> check = QVector<bool>(imax+1, false);

    for (int iLogical=0; iLogical<ToHardware.size(); iLogical++)
    {
        int iHardware = ToHardware[iLogical];
        if ( iHardware < 0 )       return "Logical channel# "+QString::number(iLogical)+" is not mapped to any hardware channel";
        if ( check.at(iHardware) ) return "Several logical channels are mapped to the same hardware one";
        check[iHardware] = true;

        if ( (iHardware >= ToLogical.size()) || (ToLogical.at(iHardware) != iLogical) )
        return "Logical channel is mapped to hardware channel which is not correctly mapped back (Logical# "+
               QString::number(iLogical)+", Hardware# "+QString::number(iHardware)+")";
    }

    imax = -1;
    for (int ich : ToLogical) if ( ich > imax ) imax = ich;
    check = QVector<bool>(imax+1, false);

    for (int iHardware=0; iHardware<ToLogical.size(); iHardware++)
    {
        int iLogical = ToLogical.at(iHardware);
        if ( iLogical < 0 ) continue;
        if ( check.at(iLogical) ) return "Several hardware channels are mapped to the same logical one";
        check[iLogical] = true;

        if ( (iLogical >= ToHardware.size()) || (ToHardware.at(iLogical) != iHardware) )
        return "Hardware channel is mapped to logical channel which is not correctly mapped back (Hardware# "+
               QString::number(iHardware)+", Logical# "+QString::number(iLogical)+")";
    }

    return "";
}
