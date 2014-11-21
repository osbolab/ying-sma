#include <sma/app/segmenter.hpp>
#include <sma/app/datalayer.hpp>
#include <sma/md5.hpp>

#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <cassert>

typedef std::string ChunkID;

std::ifstream::pos_type Segmenter::DEFAULT_CHUNK_SIZE = 1024 * 1024 * 0.5;

// Segmenter::Segmenter(ChunkStore* ptr) : chunkStorePtr(ptr) {}

void Segmenter::storeFile(std::string fileName,
                          std::vector<ChunkID>& chunks,
                          DataLayer& datalayer)
{
  std::ifstream fin(fileName.c_str(),
                    std::ios_base::ate | std::ios::binary | std::ios::in);
  if (fin.is_open()) {
    std::ifstream::pos_type nSize = fin.tellg();
    std::ostringstream oss;
    oss << "Segmenting File " << fileName << "..."
        << "Size: " << nSize << '\n';
    logger->log(oss.str());
    //    std::cout << "Segmenting File " << fileName << "..."
    //              << "Size: " << nSize << std::endl;

    fin.seekg(0, std::ios::beg);
    int currPointer = 0;
    int nChunkSize = DEFAULT_CHUNK_SIZE;
    int nPartNo = 0;

    while (fin) {
      if (currPointer >= nSize) {
        break;    // have reached the end of the file.
      }
      fin.seekg(currPointer, std::ios::beg);
      int size_to_read =
          ((currPointer + nChunkSize > nSize) ? nSize % nChunkSize :
                                                nChunkSize);
      char* memBlock = new char[size_to_read];
      fin.read(memBlock, size_to_read);
      std::string exportFileName;
      exportFileName.append(fileName);
      exportFileName.append(
          ".part");    // should be relocated to the local cache.
      std::ostringstream intToStr;
      intToStr.str("");
      intToStr << nPartNo;
      exportFileName.append(intToStr.str());

      std::ostringstream oss;
      oss << "Exporting chunk: " << exportFileName << '\n';
      logger->log(oss.str());
      //      std::cout << "Exporting chunk: " << exportFileName << std::endl;
      std::ofstream fout(exportFileName.c_str());
      fout.write(memBlock, size_to_read);
      fout.close();
      // compute the chunk ID
      MD5 md5;
      ChunkID id = md5.digestFile(exportFileName.c_str());
      chunks.push_back(id);
      // store to ChunkStore
      datalayer.storeChunk(
          exportFileName,
          false);    // not from network, but from local application layer..
      // remove temperate file
      std::remove(exportFileName.c_str());
      currPointer += size_to_read;
      nPartNo += 1;
      delete[] memBlock;
      memBlock = nullptr;
    }
    fin.close();
  } else {
    std::ostringstream oss;
    oss << "Can't open the file " << fileName << '\n';
    //    std::cout << "Can't open the so-store file..." << fileName <<
    //    std::endl;
    logger->log(oss.str());
  }
}

void Segmenter::loadFileFromChunks(std::string assembledFileName,
                                   const std::vector<ChunkID>& chunks,
                                   const DataLayer& datalayer) const
{
  std::vector<ChunkID>::const_iterator iter = chunks.begin();
  std::ofstream fout(assembledFileName.c_str());
  char* memBlock;
  while (iter != chunks.end()) {
    //    std::cout << "Reading chunk: " << *iter << std::endl;
    std::ostringstream oss;
    oss << "Reading chunk: " << *iter << '\n';
    logger->log(oss.str());
    assert(datalayer.hasChunk(*iter));
    std::ifstream fin;
    datalayer.fetchChunk(*iter, fin);
    std::ifstream::pos_type nSize = fin.tellg();
    memBlock = new char[nSize];
    fin.seekg(0, std::ios::beg);
    fin.read(memBlock, nSize);
    fin.close();
    fout.write(memBlock, nSize);
    iter++;
    delete[] memBlock;
    memBlock = nullptr;
  }
  fout.close();
}

void Segmenter::setLogger(DeviceLogger* loggerPtr)
{
  logger = loggerPtr;
}
