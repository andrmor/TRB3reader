#ifndef CHANNELMAPPER_H
#define CHANNELMAPPER_H

#include <QVector>
#include <QStringList>

class ChannelMapper
{
friend class MasterConfig;

public:
    ChannelMapper();

    int CountLogicalChannels() const {return ToHardware.size();}

      //checks the channel map for any desynchronization, or repetitions
    const QString Validate() const; // return: no error - empty QString, otherwise error string

      //safe requests
    int HardwareToLogical(int iHardwareChannel) const;
    int LogicalToHardware(int iLogicalChannel) const;
      //fast requests, no checks for validity
    int HardwareToLogicalFast(int iHardwareChannel) const;
    int LogicalToHardwareFast(int iLogicalChannel) const;

    const QVector<int>& GetMapToHardware() const {return ToHardware;}

    const QStringList PrintToLogical() const;
    const QStringList PrintToHardware() const;

protected:
    //Set channel map: vector should contain logical channel numbers for consequitive hardware channels
  //void SetChannels_OrderedByHardware(QVector<int> ToLogicalChannelMap);
    //Set channel map: vector should contain hardware channel numbers for consequitive logical channels
  void SetChannels_OrderedByLogical(QVector<int> ToHardwareChannelMap);
    //Update ToLogical after the number of hardware channels was defined in Reader/Extractor
  bool UpdateNumberOfHardwareChannels(int NewNumberOfHardwChannels);

  void Clear();

private:
    QVector<int> ToLogical;
    QVector<int> ToHardware;

    //void update_ToHardware();
    void update_ToLogical();
};

#endif // CHANNELMAPPER_H
