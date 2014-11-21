#include <sma/ccn/flowtable.hpp>
#include <unordered_map>
#include <mutex>

void FlowTable::addRule(std::string chunkID, int rule)
{
  std::unique_lock<std::mutex> lock_table (m_mutex_table, std::defer_lock);
  lock_table.lock();
  ruleTable.insert(make_pair(chunkID, rule));
  lock_table.unlock();
}

void FlowTable::delRule(std::string chunkID)
{
  std::unique_lock<std::mutex> lock_table (m_mutex_table, std::defer_lock);
  lock_table.lock();
  std::unordered_map <std::string, int>::const_iterator iter = ruleTable.find(chunkID);
  if (iter != ruleTable.end())
    ruleTable.erase(iter);
  lock_table.unlock();
}

int FlowTable::getRule (std::string chunkID) const
{
  int result = -1;
  std::unordered_map <std::string, int>::const_iterator iter = ruleTable.find(chunkID);
  if (iter != ruleTable.end())
    result = iter->second;
  return result;
}
