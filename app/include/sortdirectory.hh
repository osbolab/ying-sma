#ifndef SORT_DIRECTORY_H
#define SORT_DIRECTORY_H

#include "contentdirectory.hh"

class SortDirectory
{
public:  
  virtual bool operator() (const ContentDiscription& discriptionA, const ContentDiscription& discriptionB) = 0;
};

#endif
