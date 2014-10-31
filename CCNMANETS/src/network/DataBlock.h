#ifndef DATA_BLOCK_H
#define DATA_BLOCK_H

#include "Device.h"
#include "TypeDefinition.h"

namespace SMA
{
  enum MESSAGE_TYPE {GPSBCAST=0, REQUESTFWD = 1, CHUNK = 2, SUPER = 3, DIRBCAST=4};
  static const char* MESSAGE_TYPE_STR[] = {"GPSBCAST", "REQUESTFWD", "CHUNK", "SUPER", "DIRBCAST"};
}

class Device;

class DataBlock
{
public:
  DataBlock(SMA::MESSAGE_TYPE type);
  DataBlock(const DataBlock& block);
  DataBlock& operator= (const DataBlock& block);
  void createData(Device* devicePtr, char* carray, int size);
  ~DataBlock();
  int getPayloadSize() const;
  std::string getMsgType() const;
  void getPayload(char* outputBuffer) const;
  std::string getSrcDeviceID() const;
  SMA::MESSAGE_TYPE _getMsgTypeEnum() const;
  void setChunkID (ChunkID chunk);
  std::string getChunkID() const;
  

private:
  int payloadSize;
  SMA::MESSAGE_TYPE dataType;
  char* dataArray; 
  std::string srcDeviceID;
  std::string chunkID; //only valid if this block is a chunk block
};

#endif
