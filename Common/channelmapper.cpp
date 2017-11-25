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

void ChannelMapper::SetChannels_OrderedByLogical(QVector<int> ToHardwareChannelMap)
{
    Clear();

    int imax = -1;
    for (int ich : ToHardwareChannelMap) if (ich > imax) imax = ich;
    ToHardware = QVector<int>(imax+1, -1);

    for (int iLogical=0; iLogical<ToHardwareChannelMap.size(); iLogical++)
    {
        const int iHardware = ToHardwareChannelMap.at(iLogical);
        ToHardware[iLogical] = iHardware;
    }

    update_ToLogical();
}

void ChannelMapper::Clear()
{
    ToLogical.clear();
    ToHardware.clear();
}

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

bool ChannelMapper::Validate(int numChannels, bool ensureLogicalChannelContinuity) const
{
    if (ToHardware.isEmpty() || ToLogical.isEmpty())
    {
        qDebug() << "mapping is not defined!";
        return false;
    }

    if (ensureLogicalChannelContinuity)
        if ( ToHardware.size() != numChannels)
        {
            qDebug() << "--- ToHardware size ("<<ToHardware.size()<<") not consistent with requested number of channels ("<<numChannels<<")";
            return false;
        }

    int imax = -1;
    for (int ich : ToHardware) if ( ich > imax ) imax = ich;
    QVector<bool> check = QVector<bool>(imax+1, false);

    for (int iLogical=0; iLogical<ToHardware.size(); iLogical++)
    {
        int iHardware = ToHardware[iLogical];
        if ( iHardware < 0 )
        {
            if (ensureLogicalChannelContinuity)
            {
                qDebug() << "--- Logical channel numbering is not continuous";
                return false;
            }
            else continue;
        }
        if ( check.at(iHardware) )
        {
            qDebug() << "--- Dublication found in ToLogical ("<<iHardware<<")";
            return false;
        }
        if ( (iHardware >= ToLogical.size()) || (ToLogical[iHardware] != iLogical) )
        {
            qDebug() << "--- Not consistent ToHardware and ToLogial (Logical at"<<iLogical<<" of "<<iHardware<<")";
            return false;
        }
        check[iHardware] = true;
    }

    check = QVector<bool>(ToLogical.size(), false);
    for (int iHardware=0; iHardware<ToLogical.size(); iHardware++)
    {
        int iLogical = ToLogical.at(iHardware);
        if ( iLogical < 0 ) continue;
        if ( check.at(iLogical) )
        {
            qDebug() << "--- Dublication found in ToHardware ("<<iLogical<<")";
            return false;
        }        
        if ( (iLogical >= ToHardware.size()) || (ToHardware.at(iLogical) != iHardware) )
        {
            qDebug() << "--- Not consistent ToLogial and ToHardware (Hardware at"<<iHardware<<" of "<<iLogical<<")";
            return false;
        }
        check[iLogical] = true;
    }

    return true;
}
