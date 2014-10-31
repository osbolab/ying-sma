#include "SignalHandler.h"
#include <sstream>
#include "DataBlock.h"
#include "./json/json.h"
#include "ContentDiscription.h"
#include "DeviceLogger.h"
#include "ControlLayer.h"
#include <iostream>

//SignalHandler::SignalHandler (ControlLayer* controlPtr, DeviceLogger* loggerPtr) : control (controlPtr), logger (loggerPtr) {}

bool SignalHandler::processSignal (const DataBlock& block)
{
  bool result = true;
  if (block.getMsgType() == "GPSBCAST")  
    processBeaconing (block);
  else if (block.getMsgType() == "DIRBCAST")
    processDirectorySync (block);
  else if (block.getMsgType() == "REQUESTFWD")
    processRequestFwd (block);
  else if (block.getMsgType() == "CHUNK")
    processIncomingChunk(block);
  else
    result = false;

  return result; 
}

void SignalHandler::setControlLayer (ControlLayer* controlPtr)
{
  control = controlPtr;
}

void SignalHandler::setLogger (DeviceLogger* loggerPtr)
{
  logger = loggerPtr;
}

void SignalHandler::processBeaconing (const DataBlock& block)
{
  char* payload = new char [block.getPayloadSize()];
  block.getPayload(payload);
  std::string payloadJson(payload);
  delete [] payload;
  Json::Value parsedFromString;
  Json::Reader reader;
  bool success = reader.parse(payloadJson, parsedFromString);
  if (success )
  {

    std::ostringstream oss;
    oss << "Handling Beaconing Message:";
    oss << " the device id is " << parsedFromString["device_id"].asString();
    oss << " with GPS  " << parsedFromString["gps"]["latitude"].asDouble() << ','
        << parsedFromString["gps"]["longitude"].asDouble() << '\n';
    logger->log(oss.str());

    control->updateNeighborRecord(parsedFromString["device_id"].asString(),
                                 parsedFromString["gps"]["latitude"].asDouble(),
                                 parsedFromString["gps"]["longitude"].asDouble());
  }
}

void SignalHandler::processIncomingChunk (const DataBlock& block)
{
  std::ostringstream oss;
  oss << "Receiving chunk " << block.getChunkID() << " from "
      << block.getSrcDeviceID() << '\n';
//  oss << "The chunk size is " << block.getPayloadSize() << '\n';
  logger->log (oss.str());
  char* payload = new char [block.getPayloadSize()];
  block.getPayload(payload);

  //check flow table first
  int rule = control->getRuleFromFlowTable (block.getChunkID());

  switch (rule)
  {
    case -1: //unsolicited chunk block
      // drop
      logger->log ("Dropping unsolicited chunk block...\n");
      break; 
    case 0:
      // should be true, as requested by self.
      control->storeChunk(payload, block.getPayloadSize(), true, block.getChunkID()); 
      control->delRuleFromFlowTable (block.getChunkID());
      break;
    case 1:
      control->storeChunk(payload, block.getPayloadSize(), false, block.getChunkID());
      control->transmitChunk(block.getChunkID());
      control->delRuleFromFlowTable (block.getChunkID());
      break;
    default:
      std::ostringstream oss;
      oss << "Invalid rule type: " << rule << '\n';
      logger->log (oss.str());
  }  
  delete [] payload;
  payload = NULL;
 
}

void SignalHandler::processRequestFwd (const DataBlock& block)
{
  char* payload = new char [block.getPayloadSize()];
  block.getPayload(payload);
  std::string payloadJson(payload);
  delete [] payload;
  payload = NULL;
  Json::Value parsedFromString;
  Json::Reader reader;
  bool success = reader.parse(payloadJson, parsedFromString);
  if (success)
  {
    std::ostringstream oss;
    oss << "Handling request-forward message ";
    oss << "from device id " << block.getSrcDeviceID() 
        << ": chunk ID is "
        << parsedFromString["chunk_id"].asString() << '\n';
    logger->log(oss.str());
 //   for (unsigned int index=0; index<parsedFromString.size(); index++)
 //   {
//      control->transmitChunk(parsedFromString[index]["chunk_id"].asString());
    std::string chunkID = parsedFromString["chunk_id"].asString();
    if (control->hasChunk(chunkID))
      control->transmitChunk(parsedFromString["chunk_id"].asString());
    else
    {
      if (control->getRuleFromFlowTable (chunkID) == -1)
      {
        control->addRuleToFlowTable (chunkID, 1); //forward
        // forward request
        control->forwardRequest (chunkID);
      }
    }
//    }
  }
}

void SignalHandler::processDirectorySync (const DataBlock& block)
{
  char* payload = new char [block.getPayloadSize()];
  block.getPayload(payload);
  std::string payloadJson(payload);
  delete [] payload;
  Json::Value parsedFromString;
  Json::Reader reader;
  bool success = reader.parse(payloadJson, parsedFromString);
  if (success)
  {
    for (unsigned int index=0; index<parsedFromString.size(); index++)
    {
      ContentDiscription newEntry(parsedFromString[index]["name"].asString());
      Json::Value chunkList = parsedFromString[index]["chunk_list"];
      for (auto const& id : chunkList.getMemberNames()) 
      {
        newEntry.addNewChunk(atoi(id.c_str()), chunkList[id].asString());
      }
      Json::Value attrList = parsedFromString[index]["attr_list"];
      for (auto const& id : attrList.getMemberNames())
      {
        newEntry.addAttribute(ContentAttribute::to_META_TYPE(id.c_str()), attrList[id].asString());  
      }
      control->updateDirectory(newEntry);
    }
  }
}

