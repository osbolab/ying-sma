#pragma once


#include <mutex>
#include <string>
#include <unordered_map>
/*
 * getRule
 * =-1, no rule
 * 0, return to local node.
 * 1, broadcast
 */
class FlowTable
{
public:
  void addRule(std::string chunkID, int rule);
  void delRule(std::string chunkID);
  int getRule(std::string chunkID) const;

private:
  std::unordered_map <std::string, int> ruleTable;
  std::mutex m_mutex_table;
};


