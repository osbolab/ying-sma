#pragma once


#include <vector>
#include <sma/contentdescriptor.hpp>
#include <mutex>
#include <sma/sortdirectory.hpp>
#include <sma/segmenter.hpp>

class ContentDirectory
{
public:
  void addContentToDirectory (const ContentDescriptor& descriptor); 
  void rankDirectory(); // a naive ranking mechanism, based on the Rank field.
  std::vector<ContentDescriptor> getNDirectory(int num) const;
  std::vector<ContentDescriptor> getDirectory() const;
  std::vector<ChunkID> getChunkList(std::string fileName) const;
//  std::vector<pair<ContentAttribute::META_TYPE, std::string> > getMetaList(std::string fileName) const;
private:
  std::vector<ContentDescriptor> directory; 
  std::mutex m_mutex;  // synchronization
};


