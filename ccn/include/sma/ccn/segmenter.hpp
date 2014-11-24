#pragma once

#include <sma/ccn/chunkstore.hpp>
#include <sma/ccn/datalayer.hpp>

#include <string>
#include <fstream>
#include <vector>

class DataLayer;

class Segmenter
{
public:
  void storeFile (std::string fileName, std::vector<std::string>& chunks, DataLayer& datalayer);
  void loadFileFromChunks (std::string assembledFileName, const std::vector<std::string>& chunks, const DataLayer& datalayer) const;
private:
  static std::ifstream::pos_type DEFAULT_CHUNK_SIZE;
};


