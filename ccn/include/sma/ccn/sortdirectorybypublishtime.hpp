#pragma once


#include <sma/ccn/sortdirectory.hpp>
#include <sma/ccn/contentdescriptor.hpp>
#include <string>


class SortDirectoryByPublishTime : public SortDirectory
{
public:
  bool operator() (const ContentDescriptor& descriptorA, const ContentDescriptor& descriptorB)
  {
    return descriptorA.newerThan(descriptorB);
  }
};


