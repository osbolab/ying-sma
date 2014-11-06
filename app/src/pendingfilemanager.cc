#include "pendingfilemanager.hh"
#include <string>
#include <mutex>
#include <unordered_map>
#include <iostream>

void PendingFileManager::addTask (std::string readFileName, std::string saveFileName)
{
  this->delTask (readFileName);
  std::unique_lock<std::mutex> lock_map (m_mutex_map, std::defer_lock);
  lock_map.lock();
  fileNameMap.insert(make_pair(readFileName, saveFileName));
  lock_map.unlock();  
}

void PendingFileManager::delTask (std::string readFileName)
{
  std::unique_lock<std::mutex> lock_map (m_mutex_map, std::defer_lock);
  lock_map.lock();
  std::unordered_map<std::string, std::string>::const_iterator iter = fileNameMap.find(readFileName);
  if (iter != fileNameMap.end())
    fileNameMap.erase(iter);
  lock_map.unlock();
}

std::string PendingFileManager::getExportName(std::string readFileName) const
{
  std::string result="";
  std::unordered_map<std::string, std::string>::const_iterator iter = fileNameMap.find(readFileName);
  if (iter != fileNameMap.end())
    result = iter->second;
  return result;
}

void PendingFileManager::print() const
{
//  std::unique_lock<std::mutex> lock_map (m_mutex_map, std::defer_lock);
//  lock_map.lock();
  std::unordered_map<std::string, std::string>::const_iterator iter = fileNameMap.begin();
  while (iter != fileNameMap.end())
  {
    std::cout << iter->first << " will be saved as " << iter->second << std::endl;
    iter++;
  }
//  lock_map.unlock();  
}
