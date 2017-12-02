#ifndef CHANNELMAPPER_H
#define CHANNELMAPPER_H

#include <QVector>

class ChannelMapper
{
friend class MasterConfig;

public:
    ChannelMapper();

      //checks the channel map for any desynchronization, or repetitions
    bool Validate(int numChannels, bool ensureLogicalChannelContinuity = true) const;

      //safe requests
    int HardwareToLogical(int iHardwareChannel) const;
    int LogicalToHardware(int iLogicalChannel) const;
      //fast requests, no checks for validity
    int HardwareToLogicalFast(int iHardwareChannel) const;
    int LogicalToHardwareFast(int iLogicalChannel) const;

    const QVector<int>& GetMapToHardware() const {return ToHardware;}

protected:
    //Set channel map: vector should contain logical channel numbers for consequitive hardware channels
  void SetChannels_OrderedByHardware(QVector<int> ToLogicalChannelMap);
    //Set channel map: vector should contain hardware channel numbers for consequitive logical channels
  void SetChannels_OrderedByLogical(QVector<int> ToHardwareChannelMap);

  void Clear();

private:
    QVector<int> ToLogical;
    QVector<int> ToHardware;

    void update_ToHardware();
    void update_ToLogical();
};

#endif // CHANNELMAPPER_H
