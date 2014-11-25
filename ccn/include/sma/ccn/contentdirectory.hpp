#pragma once

#include <sma/ccn/contentdescriptor.hpp>
#include <sma/ccn/sortdirectory.hpp>
#include <sma/ccn/segmenter.hpp>

#include <sma/io/log>

#include <mutex>
#include <vector>

class ContentDirectory
{
public:
  ContentDirectory(sma::Logger log);
  void addContentToDirectory (const ContentDescriptor& descriptor);
  void rankDirectory(); // a naive ranking mechanism, based on the Rank field.
  std::vector<ContentDescriptor> getNDirectory(int num) const;
  std::vector<ContentDescriptor> getDirectory() const;
  std::vector<std::string> getChunkList(std::string fileName) const;
//  std::vector<pair<ContentAttribute::META_TYPE, std::string> > getMetaList(std::string fileName) const;
private:
  sma::Logger const log;
  std::vector<ContentDescriptor> directory;
  std::mutex m_mutex;  // synchronization
};


