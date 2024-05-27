#include "trb3timingrecord.h"
#include "masterconfig.h"

Trb3TimingRecord::Trb3TimingRecord(unsigned int boardDatakind, int internalChannel) :
    BoardDatakind(boardDatakind), InternalChannel(internalChannel) {}

void Trb3TimingRecord::updateTimingChannel(const std::vector<int> & map)
{
    //TimingCannel = InternalChannel;

    int add = 0;
    if      (BoardDatakind == 0xa003) add = 0;
    else if (BoardDatakind == 0xa004) add = 32;
    else return;

    int corrIndex = add + InternalChannel - 1;
    if (corrIndex > 63) return;
    TimingCannel = map[corrIndex];
}
