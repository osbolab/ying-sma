#pragma once


#include <sma/ccn/datablock.hpp>
#include <sma/ccn/devicelogger.hpp>
//#include <sma/ccn/controllayer.hpp>
#include <sma/ccn/devicelogger.hpp>

class ControlLayer;

class SignalHandler
{
public:
//  SignalHandler (ControlLayer* controlPtr, DeviceLogger* loggerPtr);
/* true - valid signal
 * flase - invalid signal
 */
  bool processSignal (const DataBlock& block);
  void setControlLayer (ControlLayer* controlPtr);
  void setLogger (DeviceLogger* logger);
private:
  ControlLayer* control;
  DeviceLogger* logger;
  void processBeaconing (const DataBlock& block);
  void processDirectorySync (const DataBlock& block);
  void processRequestFwd (const DataBlock& block);
  void processIncomingChunk (const DataBlock& block);
};



