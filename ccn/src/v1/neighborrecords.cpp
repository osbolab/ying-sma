#include <sma/ccn/neighborrecords.hpp>

#include <sma/chrono>
#include <sma/io/log>

#include <mutex>
#include <string>
#include <sstream>
#include <chrono>
#include <vector>

NeighborRecords::NeighborRecords(sma::Logger log)
  : log(log)
{
}

void NeighborRecords::updateRecord(std::string id,
                                   double latitude,
                                   double longitude)
{
  std::unique_lock<std::mutex> lock_records(m_mutex_records, std::defer_lock);
  lock_records.lock();
  std::unordered_map<std::string, NeighborRecordEntry>::const_iterator iter
      = records.find(id);
  bool did_erase = false;
  if (iter != records.end()) {
    records.erase(iter);
    did_erase = true;
  }
  records.insert(make_pair(id, NeighborRecordEntry(id, latitude, longitude)));
  if (!did_erase)
    log.d("discovered neighbor: %v", id);
  lock_records.unlock();
}

void NeighborRecords::getNeighborIDs(std::vector<std::string>& list) const
{
  for (auto kv : records) {
    list.push_back(kv.first);
  }
}

std::string NeighborRecords::getNeighborInfo(std::string id) const
{
  std::unordered_map<std::string, NeighborRecordEntry>::const_iterator iter
      = records.find(id);
  if (iter == records.end())
    return "";
  else {
    NeighborRecordEntry entry = iter->second;
    std::time_t t = std::chrono::system_clock::to_time_t(entry.getTimestamp());
    //    std::cout << std::ctime(&t) << std::endl;

    std::ostringstream formatOutput;
    formatOutput << entry.getDeviceID() << ":" << entry.getLatitude() << ":"
                 << entry.getLongitude() << ":"
                 << sma::chrono::system_clock::utcstrftime(
                        entry.getTimestamp());
    return formatOutput.str();
  }
}
