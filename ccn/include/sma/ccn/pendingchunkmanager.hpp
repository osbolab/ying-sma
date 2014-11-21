#pragma once


#include <set>
#include <mutex>
#include <vector>
#include <string>
#include <unordered_map>

class PendingChunkManager
{
public:
  void addDownloadTask(std::string fileName, std::string chunk);
  void completeDownloadTask(std::string chunk, std::vector<std::string> &filesCompleted);
  void printRemainingChunksOfFile(std::string fileName) const;

private:
  void removeChunkFromFile (std::string chunk, std::string fileName, bool &complete);
  std::unordered_map<std::string, std::set<std::string>* > pendingChunksInFile;
  std::unordered_map<std::string, std::set<std::string>* > pendingFilesOfChunk;
  std::mutex m_mutex_file;
  std::mutex m_mutex_chunk;
};


