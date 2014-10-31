#ifndef CONTENT_DIRECTORY_H
#define CONTENT_DIRECTORY_H

#include <vector>
#include "ContentDiscription.h"
#include <mutex>
#include "SortDirectory.h"
#include "Segmenter.h"

class ContentDirectory
{
public:
  void addContentToDirectory (const ContentDiscription& discription); 
  void rankDirectory(); // a naive ranking mechanism, based on the Rank field.
  std::vector<ContentDiscription> getNDirectory(int num) const;
  std::vector<ContentDiscription> getDirectory() const;
  std::vector<ChunkID> getChunkList(std::string fileName) const;
//  std::vector<pair<ContentAttribute::META_TYPE, std::string> > getMetaList(std::string fileName) const;
private:
  std::vector<ContentDiscription> directory; 
  std::mutex m_mutex;  // synchronization
};

#endif
