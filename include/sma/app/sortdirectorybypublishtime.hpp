#pragma once


#include "sortdirectory.hpp"
#include "contentdescriptor.hpp"
#include <string>


class SortDirectoryByPublishTime : public SortDirectory
{
public:
  bool operator() (const ContentDescriptor& descriptorA, const ContentDescriptor& descriptorB)
  {
    return descriptorA.newerThan(descriptorB);
  }
};


