#pragma once


#include "contentdirectory.hpp"

class SortDirectory
{
public:  
  virtual bool operator() (const ContentDescriptor& descriptorA, const ContentDescriptor& descriptorB) = 0;
};


