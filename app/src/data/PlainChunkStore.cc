#include "PlainChunkStore.h"
#include <cassert>
#include <iostream>
#include "../md5/MD5.h"
#include <string>
#include <fstream>
#include <mutex>
#include <cstdio>
#include <unordered_map>
#include <utility>
#include <iterator>
#include <algorithm>
#include <sstream>


//std::string PlainChunkStore::DEFAULT_DIR = "./cache/";
//std::mutex PlainChunkStore::m_mutex;
//std::unordered_map <std::string, std::string> PlainChunkStore::m_chunkIndex;

PlainChunkStore::PlainChunkStore(std::string dirName) : cacheDir(dirName) 
{
  std::ostringstream oss;
//  oss << "Trying to link to content store " << dirName << "..." << std::endl;
  std::cout << "Trying to link to content store " << dirName << "..." << std::endl;
  if (!std::ifstream(dirName))    
    std::cout << "Fail" << std::endl;
//    oss << "Fail" << std::endl;
  else                            
    std::cout << "Success" << std::endl;
//    oss << "Success" << std::endl;

//  logger->log (oss.str());
}

bool PlainChunkStore::hasChunk (std::string chunkID) const
{
  std::unordered_map<std::string, std::string>::const_iterator iter = m_chunkIndex.find(chunkID);
  return (iter != m_chunkIndex.end());
}

ChunkID PlainChunkStore::storeChunk (std::string tmpChunkFile)
{
  ChunkID result = "";
//  if (check_md5 && (chunkID != tmpChunkFile))
//  {
//    std::cerr<< "digest verification failed." << std::endl;
//    return;
//  }
  std::ifstream fin (tmpChunkFile, std::ios_base::ate|std::ios::binary|std::ios::in); 
  if (fin.is_open())
  {
    MD5 md5;
    std::string chunkID = md5.digestFile(tmpChunkFile.c_str());
    std::unique_lock<std::mutex> lock_index (m_mutex, std::defer_lock); // synchronization
    lock_index.lock();
    std::string storedAsName = cacheDir + chunkID;
    std::pair<std::string, std::string> newEntry = std::make_pair(chunkID, storedAsName); 
    m_chunkIndex.insert(newEntry);
    if (std::ifstream  (storedAsName))
    {
//      std::cout << "Chunk " << storedAsName << " exists!" << std::endl;
      std::ostringstream oss;
      oss << "Chunk " << storedAsName << " exists!" << std::endl;
//      logger->log (oss.str());
      fin.close();
      return result;
    }
    lock_index.unlock();


    std::ofstream fout(storedAsName, std::ios::binary|std::ios::out);

    fin.seekg(0, std::ios::beg);
    std::istreambuf_iterator<char> begin_source(fin);
    std::istreambuf_iterator<char> end_source;
    std::ostreambuf_iterator<char> begin_dest(fout);
    std::copy(begin_source, end_source, begin_dest);
    fin.close();
    fout.close();
    result = chunkID;
/*
    std::ofstream fout(storedAsName, std::ios_base::ate|std::ios::binary|std::ios::out);
    std::ifstream::pos_type nSize = fin.tellg();
    char* sMemBlock = new char[nSize];
    fin.seekg(0, std::ios::beg);
    fin.read(sMemBlock, nSize);
    fin.close();
    fout.write(sMemBlock, nSize);
    fout.close();
    delete[] sMemBlock; 
*/
  }
  return result;
}

void PlainChunkStore::deleteChunk (std::string chunkID)
{
  if (this->hasChunk(chunkID))
  {
     std::string filename;
     std::unique_lock<std::mutex> lock_index(m_mutex, std::defer_lock);
     lock_index.lock();
     std::unordered_map<std::string, std::string>::iterator const_iter = m_chunkIndex.find(chunkID);
     filename = const_iter->second;
     m_chunkIndex.erase(const_iter);
     lock_index.unlock();
     std::remove(filename.c_str());  // May want to check the ret value 
  }
}

void PlainChunkStore::fetchChunk(std::string chunkID, std::ifstream& fin) const
{
  if (this->hasChunk(chunkID))
  {
    std::unordered_map<std::string, std::string>::const_iterator const_iter = m_chunkIndex.find(chunkID);
    std::string filename = const_iter->second;
    fin.open(filename.c_str(), std::ios_base::ate|std::ios::in|std::ios::binary);
  } 
  else
  {
    std::ostringstream oss;
    oss << "Error occurs when fetching chunk " << chunkID << std::endl;
    logger->log (oss.str());
//    std::cerr << "Error occurs when fetching chunk " << chunkID << std::endl;
  }
}

void PlainChunkStore::setLogger (DeviceLogger* loggerPtr)
{
  logger = loggerPtr; 
}

PlainChunkStore::~PlainChunkStore() {}
