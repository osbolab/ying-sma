#ifndef SORT_DIRECTORY_BY_RANK_H
#define SORT_DIRECTORY_BY_RANK_H

#include "sortdirectory.hh"
#include "contentdiscription.hh"
#include <string>
#include <sstream>

class SortDirectoryByRank : public SortDirectory
{
public:
  bool operator() (const ContentDiscription& discriptionA, const ContentDiscription& discriptionB)
  {
    std::string sRankA = discriptionA.getTypeValue(ContentAttribute::Rank);
    std::string sRankB = discriptionB.getTypeValue(ContentAttribute::Rank);
    if (!sRankA.empty() && !sRankB.empty())
    {
      int iRankA, iRankB;
      std::istringstream(sRankA) >> iRankA;
      std::istringstream(sRankB) >> iRankB;
      return iRankA < iRankB;
    }
    else if (sRankA.empty() && !sRankB.empty())
    {
      return false;
    }
    else
    {
      return true;
    }
  }
};

#endif
