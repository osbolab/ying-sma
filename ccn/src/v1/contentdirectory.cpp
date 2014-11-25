#include <sma/ccn/contentdirectory.hpp>
#include <sma/ccn/contentdescriptor.hpp>
#include <sma/ccn/sortdirectory.hpp>
#include <sma/ccn/sortdirectorybyrank.hpp>
#include <sma/ccn/sortdirectorybypublishtime.hpp>

#include <sma/io/log>

#include <mutex>
#include <vector>
#include <algorithm>
#include <iostream>
#include <functional>


ContentDirectory::ContentDirectory(sma::Logger log)
  : log(std::move(log))
{
}

void ContentDirectory::addContentToDirectory(
    const ContentDescriptor& description)
{
  std::unique_lock<std::mutex> lock_directory(m_mutex, std::defer_lock);
  lock_directory.lock();
  // check whether the description is already in the directory
  // if yes and newer than the old record, replace.
  // otherwise, simply add it.

  std::string nameOfNewFile = description.getContentName();
  auto it = directory.begin();
  while (it != directory.end()) {
    if (it->getContentName() == nameOfNewFile) {
      if (description.newerThan(*it)) {
        it = directory.erase(it);
        directory.push_back(description);
        log.d("updated content directory entry '%v'", nameOfNewFile);
      }
      break;
    }
    ++it;
  }
  if (it == directory.end()) {
    directory.push_back(description);
    log.i("discovered content '%v'", nameOfNewFile);
  }
  lock_directory.unlock();
}

void ContentDirectory::rankDirectory()    // dummy argument
{
  std::unique_lock<std::mutex> lock_directory(m_mutex, std::defer_lock);
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
  std::vector<ContentDescriptor> result(begin, end);
  return result;
}

std::vector<ContentDescriptor> ContentDirectory::getDirectory() const
{
  return directory;
}

std::vector<std::string>
ContentDirectory::getChunkList(std::string fileName) const
{
  std::vector<ContentDescriptor>::const_iterator iter = directory.begin();
  std::vector<std::string> result;
  while (iter != directory.end()) {
    if (iter->getContentName() == fileName) {
      result = iter->getChunkList();
      break;
    }
    iter++;
  }
  return result;
}
