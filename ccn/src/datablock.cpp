#include <sma/app/datablock.hpp>
#include <sma/io/log>

#include <cstring>
#include <sma/app/device.hpp>
#include <iostream>

DataBlock::DataBlock(SMA::MESSAGE_TYPE type)
  : dataType(type)
  , dataArray(nullptr)
  , payloadSize(0)
{
}

DataBlock::DataBlock(const DataBlock& block)
{
  if (this != &block) {
    payloadSize = block.getPayloadSize();
    dataType = block._getMsgTypeEnum();
    srcDeviceID = block.getSrcDeviceID();
    chunkID = block.getChunkID();
    //    if (dataArray != nullptr)
    //    {
    //      std::cout << "trying to delete " << dataArray << std::endl;
    //      delete [] dataArray;
    //      dataArray = nullptr;
    //    }
    dataArray = new char[payloadSize];
    //    strncpy(dataArray, block.dataArray, payloadSize);
    memcpy(dataArray, block.dataArray, payloadSize);
  }
}

DataBlock& DataBlock::operator=(const DataBlock& block)
{
  if (this != &block) {
    this->payloadSize = block.getPayloadSize();
    this->dataType = block._getMsgTypeEnum();
    this->srcDeviceID = block.getSrcDeviceID();
    if (this->dataArray != nullptr)
      delete[] this->dataArray;
    this->dataArray = new char[payloadSize];
    //    strncpy(this->dataArray, block.dataArray, this->payloadSize);
    memcpy(this->dataArray, block.dataArray, this->payloadSize);
  }
  return *this;
}

DataBlock::~DataBlock()
{
  delete[] dataArray;
  dataArray = nullptr;
}

void DataBlock::createData(Device* devicePtr, const char* csrc, int size)
{
  srcDeviceID = devicePtr->getDeviceID();
  payloadSize = size;
  if (dataArray != nullptr)
    delete[] dataArray;
  dataArray = new char[payloadSize];
  memcpy(dataArray, csrc, size);
}

int DataBlock::getPayloadSize() const { return payloadSize; }

std::string DataBlock::getMsgType() const
{
  return SMA::MESSAGE_TYPE_STR[dataType];
}

SMA::MESSAGE_TYPE DataBlock::_getMsgTypeEnum() const { return dataType; }

void DataBlock::getPayload(char* outputBuffer) const
{
  //  if (outputBuffer != nullptr) strncpy(outputBuffer, dataArray,
  //  payloadSize);
  if (outputBuffer != nullptr)
    memcpy(outputBuffer, dataArray, payloadSize);
}

std::string DataBlock::getSrcDeviceID() const { return srcDeviceID; }

void DataBlock::setChunkID(ChunkID chunk) { chunkID = chunk; }

ChunkID DataBlock::getChunkID() const { return chunkID; }
