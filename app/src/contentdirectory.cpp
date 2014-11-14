#include <sma/app/contentdirectory.hpp>
#include <sma/app/contentdescriptor.hpp>
#include <mutex>
#include <algorithm>
#include <sma/app/sortdirectory.hpp>
#include <vector>
#include <functional>
#include <sma/app/sortdirectorybyrank.hpp>
#include <sma/app/sortdirectorybypublishtime.hpp>
#include <iostream>

void ContentDirectory::addContentToDirectory (const ContentDescriptor& description)
{
  std::unique_lock<std::mutex> lock_directory (m_mutex, std::defer_lock);
  lock_directory.lock();
  // check whether the description is already in the directory
  // if yes and newer than the old record, replace.
  // otherwise, simply add it.

  std::string nameOfNewFile = description.getContentName();
  std::vector<ContentDescriptor>::iterator iter = directory.begin();
  while (iter != directory.end())
  {
    if (iter->getContentName() == nameOfNewFile)
    {
      if (description.newerThan(*iter))
      {
//        std::cout << "updating directory entry..." << std::endl;
        directory.erase(iter);
        directory.push_back(description);
      }
//      std::cout << "not updated..." << std::endl;
      break;
    }
    iter++;
  }
  if (iter == directory.end())
    directory.push_back(description);
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



std::vector<ContentDescriptor> ContentDirectory::getNDirectory(int num) const
{
  num = (num > directory.size() ? directory.size() : num);
  std::vector<ContentDescriptor>::const_iterator begin = directory.begin();
  std::vector<ContentDescriptor>::const_iterator end = directory.begin() + num;
  std::vector<ContentDescriptor> result (begin, end);
  return result;
}

std::vector<ContentDescriptor> ContentDirectory::getDirectory() const
{
  return directory;
}

std::vector<ChunkID> ContentDirectory::getChunkList(std::string fileName) const
{
  std::vector<ContentDescriptor>::const_iterator iter = directory.begin();
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


