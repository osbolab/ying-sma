#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include "datablock.hh"
#include "devicelogger.hh"
//#include "controllayer.hh"
#include "devicelogger.hh"

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


#endif
