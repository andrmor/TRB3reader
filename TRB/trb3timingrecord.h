#ifndef TRB3TIMINGRECORD_H
#define TRB3TIMINGRECORD_H

#include <vector>

class Trb3TimingRecord
{
public:
    Trb3TimingRecord(unsigned boardDatakind, int internalChannel);

    unsigned            BoardDatakind;
    int                 InternalChannel;
    int                 TimingCannel;     // global timing channel over all boards (starts from 0 and consequitive; only if appears in "Time" save config)
    std::vector<double> Triggers;         // in ns

    void updateTimingChannel(const std::vector<int> & map);
};

#endif // TRB3TIMINGRECORD_H
