#ifndef FLOW_TABLE_H
#define FLOW_TABLE_H

#include <unordered_map>
#include <mutex>
#include "TypeDefinition.hh"
#include <string>
/*
 * getRule
 * =-1, no rule
 * 0, return to local node.
 * 1, broadcast
 */
class FlowTable
{
public:
  void addRule(ChunkID chunkID, int rule);
  void delRule(ChunkID chunkID);
  int getRule(ChunkID chunkID) const;
 
private:
  std::unordered_map <ChunkID, int> ruleTable;
  std::mutex m_mutex_table;
};

#endif
