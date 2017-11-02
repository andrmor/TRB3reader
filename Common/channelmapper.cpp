#include "channelmapper.h"

#include <iostream>
#include <limits>
#include <cmath>

using namespace std;

const size_t NaN = std::numeric_limits<size_t>::quiet_NaN();

ChannelMapper::ChannelMapper() {}

size_t ChannelMapper::HardwareToLogical(size_t HardwareChannel) const
{
    if (HardwareChannel<ToLogical.size())
        return ToLogical[HardwareChannel];
    std::cout << "--- Invalid hardware channel!\n"<<std::flush;
    return NaN;
}

size_t ChannelMapper::LogicalToHardware(size_t LogicalChannel) const
{
    if (LogicalChannel<ToLogical.size())
        return ToHardware[LogicalChannel];
    std::cout << "--- Invalid logical channel!\n"<<std::flush;
    return NaN;
}

size_t ChannelMapper::HardwareToLogicalFast(size_t HardwareChannel) const
{
    return ToLogical[HardwareChannel];
}

size_t ChannelMapper::LogicalToHardwareFast(size_t LogicalChannel) const
{
    return ToHardware[LogicalChannel];
}

void ChannelMapper::SetChannels_OrderedByHardware(std::vector<size_t> ToLogicalChannelMap)
{
    Clear();

    for (size_t iHardware=0; iHardware<ToLogicalChannelMap.size(); iHardware++)
    {
        size_t iLogical = ToLogicalChannelMap[iHardware];
        if (iHardware >= ToLogical.size()) ToLogical.resize(iHardware + 1, NaN);
        ToLogical[iHardware] = iLogical;
    }

    update_ToHardware();
}

void ChannelMapper::SetChannels_OrderedByLogical(std::vector<size_t> ToHardwareChannelMap)
{
    Clear();

    for (size_t iLogical=0; iLogical<ToHardwareChannelMap.size(); iLogical++)
    {
        size_t iHardware = ToHardwareChannelMap[iLogical];
        if (iLogical >= ToHardware.size()) ToHardware.resize(iLogical + 1, NaN);
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

    for (size_t iHardware=0; iHardware<ToLogical.size(); iHardware++)
    {
        size_t iLogical = ToLogical[iHardware];
        if ( isnan(iLogical) ) continue;
        if (iLogical>=ToHardware.size()) ToHardware.resize(iLogical+1, NaN);
        ToHardware[ iLogical ] = iHardware;
    }
}

void ChannelMapper::update_ToLogical()
{
    ToLogical.clear();

    for (size_t iLogical=0; iLogical<ToHardware.size(); iLogical++)
    {
        size_t iHardware = ToHardware[iLogical];
        if ( isnan(iHardware) ) continue;
        if (iHardware>=ToLogical.size()) ToLogical.resize(iHardware+1, NaN);
        ToLogical[ iHardware ] = iLogical;
    }
}

bool ChannelMapper::Validate(size_t numChannels, bool ensureLogicalChannelContinuity) const
{
    if (ensureLogicalChannelContinuity)
        if ( ToHardware.size() != numChannels)
        {
            std::cout << "--- ToHardware size ("<<ToHardware.size()<<") not consistent with requested number of channels ("<<numChannels<<")\n"<<std::flush;
            return false;
        }

    std::vector<bool> check;
    for (size_t iLogical=0; iLogical<ToHardware.size(); iLogical++)
    {
        size_t iHardware = ToHardware[iLogical];
        if ( isnan(iHardware) )
        {
            if (ensureLogicalChannelContinuity)
            {
                std::cout << "--- Logical channel numbering is not continuous\n"<<std::flush;
                return false;
            }
            else continue;

        }
        if ( iHardware >= check.size()) check.resize(iHardware+1, false);
        if (check[iHardware])
        {
            std::cout << "--- Dublication found in ToLogical ("<<iHardware<<")\n"<<std::flush;
            return false;
        }
        if ( (iHardware >= ToLogical.size()) || (ToLogical[iHardware] != iLogical) )
        {
            std::cout << "--- Not consistent ToHardware and ToLogial (Logical at"<<iLogical<<" of "<<iHardware<<")\n"<<std::flush;
            return false;
        }
    }

    check.resize(ToLogical.size(), false);
    for (size_t iHardware=0; iHardware<ToLogical.size(); iHardware++)
    {
        size_t iLogical = ToLogical[iHardware];
        if ( isnan(iLogical) ) continue;
        if (check[iLogical])
        {
            std::cout << "--- Dublication found in ToHardware ("<<iLogical<<")\n"<<std::flush;
            return false;
        }
        if ( (iLogical >= ToHardware.size()) || (ToHardware[iLogical] != iHardware) )
        {
            std::cout << "--- Not consistent ToLogial and ToHardware (Hardware at"<<iHardware<<" of "<<iLogical<<")\n"<<std::flush;
            return false;
        }
    }

    return true;
}
