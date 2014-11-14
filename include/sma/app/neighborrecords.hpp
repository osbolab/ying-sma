#pragma once


#include <unordered_map>
#include <mutex>
#include <string>
#include "neighborrecordentry.hpp"
#include <vector>

class NeighborRecords
{
  public:
    void updateRecord(std::string id, double latitude, double longitude); 
    void getNeighborIDs(std::vector<std::string>& list) const;
    std::string getNeighborInfo(std::string id) const;
  private:
    std::unordered_map<std::string, NeighborRecordEntry> records; 
    std::mutex m_mutex_records;
    //shoudl have a private method to delete thos outdated record, like 10 mins ago.
};


