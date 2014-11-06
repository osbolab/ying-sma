#ifndef SORT_DIRECTORY_BY_PUBLISHTIME_H
#define SORT_DIRECTORY_BY_PUBLISHTIME_H

#include "sortdirectory.hh"
#include "contentdiscription.hh"
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
