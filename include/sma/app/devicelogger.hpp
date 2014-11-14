#pragma once


#include <fstream>
#include <mutex>
#include <string>

class DeviceLogger
{

public:
  DeviceLogger (std::string fileName);
  void log (std::string content);
  void close();

private:
  std::ofstream logFile;
  std::mutex m_mutex_log;
};


