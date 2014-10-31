#ifndef SORT_DIRECTORY_BY_PUBLISHTIME_H
#define SORT_DIRECTORY_BY_PUBLISHTIME_H

#include "SortDirectory.h"
#include "ContentDiscription.h"
#include <string>


class SortDirectoryByPublishTime : public SortDirectory
{
public:
  bool operator() (const ContentDiscription& discriptionA, const ContentDiscription& discriptionB)
  {
    return discriptionA.newerThan(discriptionB);
  }
};

#endif
