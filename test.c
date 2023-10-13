#include "hadaq/api.h"

#include <string>
#include <iostream>

int main(int argc, char** argv)
{
    std::string fileName = "/home/andr/Downloads/dabc19214020530.hld";
    hadaq::ReadoutHandle ref = hadaq::ReadoutHandle::Connect(fileName);
    hadaq::RawEvent* evnt = 0;

    int numEvents = 0;
    while ( (evnt = ref.NextEvent(1.0)) != nullptr )
    {
        std::cout << std::endl << "Event #" << numEvents++;
        hadaq::RawSubevent * sub = nullptr;
        while ( (sub = evnt->NextSubevent(sub)) != nullptr)
        {
            unsigned trbSubEvSize = sub->GetSize() / 4 - 4;
            unsigned ix = 0;

            while (ix < trbSubEvSize)
            {
                unsigned hadata   = sub->Data(ix++);
                unsigned datalen  = (hadata >> 16) & 0xFFFF;
                int      datakind = hadata & 0xFFFF;

                unsigned ixTmp = ix;   // -->

                if (datakind == 0xA001 || datakind == 0xA004)
                {
                    unsigned lastword   = sub->Data(ix+datalen-1);
                    int      ch_per_adc = ((lastword >> 16) & 0xF) + 1;
                    int      n_adcs     = ((lastword >> 20) & 0xF) + 1;
                    int      channels   = ch_per_adc * n_adcs;
                    int      samples    = datalen/channels;

                    std::cout << std::endl << "Channels: "<< channels << " Samples: " << samples << std::endl;

                    for (int iChan = 0; iChan < channels; iChan++)
                    {
                        for (int iSample = 0; iSample < samples; iSample++)
                            std::cout << (sub->Data(ix++) & 0xFFFF) << " ";
                        std::cout << std::endl;
                    }
                }

                ix = ixTmp + datalen; // <--
            }
        }
    }

    ref.Disconnect();
    std::cout << std::endl << "-> Num all events:" << numEvents << std::endl;
}
