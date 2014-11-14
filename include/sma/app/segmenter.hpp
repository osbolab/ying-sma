#pragma once


#include <string>
#include <fstream>
#include <vector>
#include <sma/chunkstore.hpp>
#include <sma/datalayer.hpp>
#include <sma/typedefinition.hpp>
#include <sma/devicelogger.hpp>

class DataLayer;

class Segmenter
{
public:
  void storeFile (std::string fileName, std::vector<ChunkID>& chunks, DataLayer& datalayer);
  void loadFileFromChunks (std::string assembledFileName, const std::vector<ChunkID>& chunks, const DataLayer& datalayer) const;
  void setLogger (DeviceLogger* logger);
private:
  DeviceLogger* logger;
  static std::ifstream::pos_type DEFAULT_CHUNK_SIZE;
};


