#pragma once

#include <sma/ccn/datablock.hpp>

#include <sma/io/log>

class ControlLayer;

class SignalHandler
{
public:
  SignalHandler(sma::Logger log, ControlLayer* cl);
  bool processSignal (const DataBlock& block);
private:
  sma::Logger log;
  ControlLayer* control;
  void processBeaconing (const DataBlock& block);
  void processDirectorySync (const DataBlock& block);
  void processRequestFwd (const DataBlock& block);
  void processIncomingChunk (const DataBlock& block);
};



