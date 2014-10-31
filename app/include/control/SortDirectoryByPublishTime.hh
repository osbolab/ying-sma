#ifndef SORT_DIRECTORY_BY_PUBLISHTIME_H
#define SORT_DIRECTORY_BY_PUBLISHTIME_H

#include "control/SortDirectory.hh"
#include "control/ContentDiscription.hh"
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
