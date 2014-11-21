#pragma once

#include <sma/ccn/contentdescriptor.hpp>
#include <sma/ccn/sortdirectory.hpp>
#include <sma/ccn/segmenter.hpp>

#include <mutex>
#include <vector>

class ContentDirectory
{
public:
  void addContentToDirectory (const ContentDescriptor& descriptor);
  void rankDirectory(); // a naive ranking mechanism, based on the Rank field.
  std::vector<ContentDescriptor> getNDirectory(int num) const;
  std::vector<ContentDescriptor> getDirectory() const;
  std::vector<std::string> getChunkList(std::string fileName) const;
//  std::vector<pair<ContentAttribute::META_TYPE, std::string> > getMetaList(std::string fileName) const;
private:
  std::vector<ContentDescriptor> directory;
  std::mutex m_mutex;  // synchronization
};


