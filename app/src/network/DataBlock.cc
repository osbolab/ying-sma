#include "DataBlock.hh"

#include <cstring>
#include "Device.hh"
#include <iostream>

DataBlock::DataBlock(SMA::MESSAGE_TYPE type):dataType(type), dataArray(NULL), payloadSize(0) {}

DataBlock::DataBlock(const DataBlock& block)
{
  if (this != &block)
  {
    payloadSize = block.getPayloadSize();
    dataType = block._getMsgTypeEnum();
    srcDeviceID = block.getSrcDeviceID();
    chunkID = block.getChunkID();
//    if (dataArray != NULL) 
//    {
//      std::cout << "trying to delete " << dataArray << std::endl;
//      delete [] dataArray;
//      dataArray = NULL;
//    }
    dataArray = new char [payloadSize];
//    strncpy(dataArray, block.dataArray, payloadSize);
    memcpy(dataArray, block.dataArray, payloadSize);
  }
}

DataBlock& DataBlock::operator= (const DataBlock& block)
{
  if (this != &block)
  {
    this->payloadSize = block.getPayloadSize();
    this->dataType = block._getMsgTypeEnum();
    this->srcDeviceID = block.getSrcDeviceID();
    if (this->dataArray != NULL) delete [] this->dataArray;
    this->dataArray = new char [payloadSize];
//    strncpy(this->dataArray, block.dataArray, this->payloadSize);
    memcpy(this->dataArray, block.dataArray, this->payloadSize);
  }
  return *this;
}

DataBlock::~DataBlock()
{
  delete [] dataArray;
  dataArray = NULL;
}

void DataBlock::createData(Device* devicePtr, char* carray, int size)
{
  srcDeviceID = devicePtr->getDeviceID();
  payloadSize = size;
  if (dataArray != NULL)  delete [] dataArray;
  dataArray = new char [payloadSize];
  memcpy(dataArray, carray, size);
}

int DataBlock::getPayloadSize() const
{
  return payloadSize;
}

std::string DataBlock::getMsgType() const
{
  return SMA::MESSAGE_TYPE_STR[dataType];
}

SMA::MESSAGE_TYPE DataBlock::_getMsgTypeEnum() const
{
  return dataType;
}

void DataBlock::getPayload(char* outputBuffer) const
{
//  if (outputBuffer != NULL) strncpy(outputBuffer, dataArray, payloadSize);
  if (outputBuffer != NULL) memcpy(outputBuffer, dataArray, payloadSize);
}

std::string DataBlock::getSrcDeviceID() const
{
  return srcDeviceID;
}

void DataBlock::setChunkID (ChunkID chunk)
{
  chunkID = chunk;
}

ChunkID DataBlock::getChunkID() const
{
  return chunkID;
}
