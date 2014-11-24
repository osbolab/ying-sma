#include <sma/ccn/signalhandler.hpp>

#include <sma/ccn/datablock.hpp>
#include <sma/ccn/contentdescriptor.hpp>
#include <sma/ccn/controllayer.hpp>
#include <sma/json.hpp>

#include <sma/io/log>

#include <sstream>
#include <iostream>


SignalHandler::SignalHandler(sma::Logger log, ControlLayer* cl)
  : log(log)
  , control(cl)
{
}

bool SignalHandler::processSignal(const DataBlock& block)
{
  bool result = true;
  if (block.getMsgType() == "GPSBCAST")
    processBeaconing(block);
  else if (block.getMsgType() == "DIRBCAST")
    processDirectorySync(block);
  else if (block.getMsgType() == "REQUESTFWD")
    processRequestFwd(block);
  else if (block.getMsgType() == "CHUNK")
    processIncomingChunk(block);
  else
    result = false;

  return result;
}

void SignalHandler::processBeaconing(const DataBlock& block)
{
  char* payload = new char[block.getPayloadSize()];
  block.getPayload(payload);
  std::string payloadJson(payload);
  delete[] payload;
  Json::Value fields;
  Json::Reader reader;
  bool success = reader.parse(payloadJson, fields);
  if (success) {
    auto sender = fields["device_id"].asString();
    auto lon = fields["gps"]["longitude"].asDouble();
    auto lat = fields["gps"]["latitude"].asDouble();

    LOG(DEBUG) << "<-- beacon { from: " << sender << ", gps: (" << lon << ", "
               << lat << ") }";

    control->updateNeighborRecord(sender, lat, lon);
  }
}

void SignalHandler::processIncomingChunk(const DataBlock& block)
{
  LOG(DEBUG) << "<-- chunk { from: " << block.getSrcDeviceID()
             << ", chunk: " << block.getChunkID() << " ("
             << block.getPayloadSize() << " bytes) }";
  char* payload = new char[block.getPayloadSize()];
  block.getPayload(payload);

  // check flow table first
  int rule = control->getRuleFromFlowTable(block.getChunkID());

  switch (rule) {
    case -1:
      LOG(WARNING) << "dropped unsolicited chunk";
      break;
    case 0:
      // should be true, as requested by self.
      control->storeChunk(
          payload, block.getPayloadSize(), true, block.getChunkID());
      control->delRuleFromFlowTable(block.getChunkID());
      break;
    case 1:
      control->storeChunk(
          payload, block.getPayloadSize(), false, block.getChunkID());
      control->transmitChunk(block.getChunkID());
      control->delRuleFromFlowTable(block.getChunkID());
      break;
    default:
      std::ostringstream oss;
      LOG(ERROR) << "invalid chunk handling rule: " << rule;
  }
  delete[] payload;
  payload = nullptr;
}

void SignalHandler::processRequestFwd(const DataBlock& block)
{
  char* payload = new char[block.getPayloadSize()];
  block.getPayload(payload);
  std::string payloadJson(payload);
  delete[] payload;
  payload = nullptr;
  Json::Value fields;
  Json::Reader reader;
  bool success = reader.parse(payloadJson, fields);
  if (success) {
    std::string chunkID = fields["chunk_id"].asString();
    LOG(DEBUG) << "<-- request-forward { from: " << block.getSrcDeviceID()
               << ", chunk: " << chunkID << " }";
    if (control->hasChunk(chunkID))
      control->transmitChunk(chunkID);
    else {
      if (control->getRuleFromFlowTable(chunkID) == -1) {
        control->addRuleToFlowTable(chunkID, 1);
        control->forwardRequest(chunkID);
      }
    }
  }
}

void SignalHandler::processDirectorySync(const DataBlock& block)
{
  char* payload = new char[block.getPayloadSize()];
  block.getPayload(payload);
  std::string payloadJson(payload);
  delete[] payload;
  Json::Value fields;
  Json::Reader reader;
  bool success = reader.parse(payloadJson, fields);
  if (success) {
    for (unsigned int index = 0; index < fields.size(); index++) {
      ContentDescriptor newEntry(fields[index]["name"].asString());
      Json::Value chunkList = fields[index]["chunk_list"];
      for (auto const& id : chunkList.getMemberNames()) {
        newEntry.addNewChunk(atoi(id.c_str()), chunkList[id].asString());
      }
      Json::Value attrList = fields[index]["attr_list"];
      for (auto const& id : attrList.getMemberNames()) {
        newEntry.addAttribute(ContentAttribute::to_META_TYPE(id.c_str()),
                              attrList[id].asString());
      }
      control->updateDirectory(newEntry);
    }
  }
}
