#include "contentdirectory.hh"
#include "contentdiscription.hh"
#include <mutex>
#include <algorithm>
#include "sortdirectory.hh"
#include <vector>
#include <functional>
#include "sortdirectorybyrank.hh"
#include "sortdirectorybypublishtime.hh"
#include <iostream>

void ContentDirectory::addContentToDirectory (const ContentDiscription& discription)
{
  std::unique_lock<std::mutex> lock_directory (m_mutex, std::defer_lock);
  lock_directory.lock();
  // check whether the discription is already in the directory
  // if yes and newer than the old record, replace.
  // otherwise, simply add it.
  
  std::string nameOfNewFile = discription.getContentName();
  std::vector<ContentDiscription>::iterator iter = directory.begin();
  while (iter != directory.end())
  {
    if (iter->getContentName() == nameOfNewFile)
    {
      if (discription.newerThan(*iter))
      {
//        std::cout << "updating directory entry..." << std::endl;
        directory.erase(iter);
        directory.push_back(discription);
      }
//      std::cout << "not updated..." << std::endl;
      break;
    } 
    iter++;
  }
  if (iter == directory.end())
    directory.push_back(discription);
  lock_directory.unlock();
}

void ContentDirectory::rankDirectory()  //dummy argument
{
  std::unique_lock<std::mutex> lock_directory (m_mutex, std::defer_lock);
  lock_directory.lock();
//  std::sort(directory.begin(), directory.end(), SortDirectoryByRank()); 
  std::sort(directory.begin(), directory.end(), SortDirectoryByPublishTime());
  lock_directory.unlock(); 
}



std::vector<ContentDiscription> ContentDirectory::getNDirectory(int num) const
{
  num = (num > directory.size() ? directory.size() : num);
  std::vector<ContentDiscription>::const_iterator begin = directory.begin();
  std::vector<ContentDiscription>::const_iterator end = directory.begin() + num;
  std::vector<ContentDiscription> result (begin, end);
  return result;
}

std::vector<ContentDiscription> ContentDirectory::getDirectory() const
{
  return directory;
}

std::vector<ChunkID> ContentDirectory::getChunkList(std::string fileName) const
{
  std::vector<ContentDiscription>::const_iterator iter = directory.begin();
  std::vector<ChunkID> result;
  while (iter != directory.end())
  {
    if (iter->getContentName() == fileName)
    {
      result = iter->getChunkList();
      break;
    }
    iter++;
  }
  return result;
}


