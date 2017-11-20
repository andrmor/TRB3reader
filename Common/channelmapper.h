#ifndef CHANNELMAPPER_H
#define CHANNELMAPPER_H

#include <vector>

class ChannelMapper
{
friend class MasterConfig;

public:
    ChannelMapper();

      //checks the channel map for any desynchronization, or repetitions
    bool Validate(std::size_t numChannels, bool ensureLogicalChannelContinuity = true) const;

      //safe requests
    std::size_t HardwareToLogical(std::size_t HardwareChannel) const;
    std::size_t LogicalToHardware(std::size_t LogicalChannel) const;
      //fast requests, no checks for validity
    std::size_t HardwareToLogicalFast(std::size_t HardwareChannel) const;
    std::size_t LogicalToHardwareFast(std::size_t LogicalChannel) const;

    std::size_t GetNumLogicalChannels() const {return ToHardware.size();}

    const std::vector<std::size_t>& GetMapToHardware() {return ToHardware;}

protected:
    //Set channel map: vector should contain logical channel numbers for consequitive hardware channels
  void SetChannels_OrderedByHardware(std::vector<std::size_t> ToLogicalChannelMap);
    //Set channel map: vector should contain hardware channel numbers for consequitive logical channels
  void SetChannels_OrderedByLogical(std::vector<std::size_t> ToHardwareChannelMap);

  void Clear();



private:
    std::vector<std::size_t> ToLogical;
    std::vector<std::size_t> ToHardware;

    void update_ToHardware();
    void update_ToLogical();
};

#endif // CHANNELMAPPER_H
