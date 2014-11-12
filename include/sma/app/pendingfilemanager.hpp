#ifndef PENDING_FILE_MANAGER_H
#define PENDING_FILE_MANAGER_H

#include <string>
#include <unordered_map>
#include <mutex>

class PendingFileManager
{
public:
  void addTask (std::string readFileName, std::string saveFileName);
  std::string getExportName(std::string readFileName) const;
  void delTask (std::string readFileName); 
  void print() const;

private:
  std::unordered_map <std::string, std::string> fileNameMap;
  std::mutex m_mutex_map;
};

#endif
