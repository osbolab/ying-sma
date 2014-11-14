#pragma once


#include <sma/sortdirectory.hpp>
#include <sma/contentdescriptor.hpp>
#include <string>
#include <sstream>

class SortDirectoryByRank : public SortDirectory
{
public:
  bool operator() (const ContentDescriptor& descriptorA, const ContentDescriptor& descriptorB)
  {
    std::string sRankA = descriptorA.getTypeValue(ContentAttribute::Rank);
    std::string sRankB = descriptorB.getTypeValue(ContentAttribute::Rank);
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


