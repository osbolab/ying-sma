#include "PendingChunkManager.hh"
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>
#include <set>
#include <iostream>
#include <mutex>
#include "TypeDefinition.hh"

void PendingChunkManager::addDownloadTask(std::string fileName, ChunkID chunk)
{
  std::unique_lock<std::mutex> lock_pending_chunks_in_file (m_mutex_file, std::defer_lock);
  lock_pending_chunks_in_file.lock();
  std::unordered_map<std::string, std::set<ChunkID>* >::const_iterator file_iter = pendingChunksInFile.find(fileName);
  if (file_iter == pendingChunksInFile.end())
  {
    std::set<ChunkID>* chunkSetPtr = new std::set<ChunkID>();
    chunkSetPtr->insert(chunk);
    pendingChunksInFile.insert(make_pair(fileName, chunkSetPtr));
  } 
  else
  {
    file_iter->second->insert(chunk);
  } 
  lock_pending_chunks_in_file.unlock(); 

  //update pendingFileOfChunks
  std::unique_lock<std::mutex> lock_pending_files_of_chunk (m_mutex_chunk, std::defer_lock);
  lock_pending_files_of_chunk.lock();
  std::unordered_map<ChunkID, std::set<std::string>* >::const_iterator chunk_iter = pendingFilesOfChunk.find(chunk);
  if (chunk_iter == pendingFilesOfChunk.end()) 
  {
    std::set<std::string>* fileSetPtr = new std::set<std::string>();
    fileSetPtr->insert(fileName);
    pendingFilesOfChunk.insert(make_pair(chunk, fileSetPtr));
  }
  else
  {
    chunk_iter->second->insert(fileName);
  }
  lock_pending_files_of_chunk.unlock();
}


/*This method is called whenever a chunk arrives and the flow table rule is 0
 */
void PendingChunkManager::completeDownloadTask(ChunkID chunk, std::vector<std::string> &filesCompleted)
{
  std::unique_lock<std::mutex> lock_pending_files_of_chunk (m_mutex_chunk, std::defer_lock);
  lock_pending_files_of_chunk.lock();
  std::unordered_map<ChunkID, std::set<std::string>* >::const_iterator chunk_iter = pendingFilesOfChunk.find(chunk);
  if (chunk_iter != pendingFilesOfChunk.end()) 
  {
    std::set<std::string>* files = chunk_iter->second;
    std::set<std::string>::iterator file_iter = files->begin();
    while (file_iter != files->end())
    {
      bool complete = false;
      removeChunkFromFile(chunk, *file_iter, complete);
      if (complete)
      {
        filesCompleted.push_back(*file_iter); 
      }
      files->erase (*file_iter++);
    }
    if (files != NULL && files->size() == 0)
    {
      delete files; 
      files = NULL;
    }
    pendingFilesOfChunk.erase(chunk_iter);
  }
  lock_pending_files_of_chunk.unlock();
}

void PendingChunkManager::removeChunkFromFile(ChunkID chunk, std::string fileName, bool &complete)
{
  std::unique_lock<std::mutex> lock_pending_chunks_in_file (m_mutex_file, std::defer_lock);
  lock_pending_chunks_in_file.lock();
  std::unordered_map<std::string, std::set<ChunkID>* >::const_iterator file_iter = pendingChunksInFile.find(fileName);
  if (file_iter != pendingChunksInFile.end())
  {
    std::set<ChunkID>* chunks = file_iter->second; 
    chunks->erase(chunk);
    if (chunks->size() == 0)
    {
      delete chunks;
      chunks = NULL;
      pendingChunksInFile.erase(file_iter); //if all chunks have been downloaded.
      complete = true;
    }
  } 
  lock_pending_chunks_in_file.unlock();
}

void PendingChunkManager::printRemainingChunksOfFile(std::string fileName) const
{
//  std::unique_lock<std::mutex> lock_pending_chunks_in_file (m_mutex_file, std::defer_lock);
//  lock_pending_chunks_in_file.lock();
  std::unordered_map<ChunkID, std::set<std::string>* >::const_iterator file_iter = pendingChunksInFile.find(fileName);
  if (file_iter != pendingChunksInFile.end())
  {
    std::set<ChunkID>* chunkSetPtr = file_iter->second;
    std::set<ChunkID>::const_iterator chunk_iter = chunkSetPtr->begin();
    while (chunk_iter != chunkSetPtr->end())
    {
      std::cout << *chunk_iter << std::endl; 
      chunk_iter++;
    }
  }
//  lock_pending_chunks_in_file.unlock(); 
}
