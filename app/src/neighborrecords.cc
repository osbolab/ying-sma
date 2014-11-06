#include "neighborrecords.hh"
#include <mutex>
#include <string>
#include <sstream>
#include <chrono>
#include <vector>

void NeighborRecords::updateRecord(std::string id, double latitude, double longitude)
{
  std::unique_lock<std::mutex> lock_records (m_mutex_records, std::defer_lock);
  lock_records.lock();
  std::unordered_map<std::string, NeighborRecordEntry>::const_iterator iter = records.find(id);
  if (iter != records.end())
  {
    records.erase(iter);
  }
  records.insert(make_pair(id, NeighborRecordEntry(id, latitude, longitude)));
  lock_records.unlock();
}

void NeighborRecords::getNeighborIDs(std::vector<std::string>& list) const
{
  for(auto kv : records) 
  {
    list.push_back(kv.first);
  } 
}

std::string NeighborRecords::getNeighborInfo(std::string id) const
{
  std::unordered_map<std::string, NeighborRecordEntry>::const_iterator iter = records.find(id);
  if (iter == records.end())  
    return "";
  else 
  {
    NeighborRecordEntry entry = iter->second;
    std::time_t t = std::chrono::system_clock::to_time_t(entry.getTimestamp());
//    std::cout << std::ctime(&t) << std::endl;

    std::ostringstream formatOutput;
    formatOutput << entry.getDeviceID() <<":"
                 << entry.getLatitude() <<":"<<entry.getLongitude() <<":"
                 << std::ctime(&t); 
    return formatOutput.str();
  } 
}
