#pragma once


#include <sma/app/datablock.hpp>
#include <sma/app/devicelogger.hpp>
//#include <sma/app/controllayer.hpp>
#include <sma/app/devicelogger.hpp>

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



