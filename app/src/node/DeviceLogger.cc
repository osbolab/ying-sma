#include "node/DeviceLogger.hh"
#include <fstream>
#include <mutex>
#include <string>
#include <sstream>
#include <ctime>
#include <chrono>
#include <iomanip>

DeviceLogger::DeviceLogger(std::string logFileName)
{
  logFile.open(logFileName.c_str());
}

void DeviceLogger::close()
{
  logFile.close();
}
void DeviceLogger::log(std::string content)
{
  std::unique_lock<std::mutex> lockLog(m_mutex_log, std::defer_lock);
  lockLog.lock();
  std::chrono::system_clock::time_point timestamp =
      std::chrono::system_clock::now();
  std::time_t t = std::chrono::system_clock::to_time_t(timestamp);
  struct std::tm* gmtm = std::gmtime(&t);
  auto duration = timestamp.time_since_epoch();
  auto mseconds =
      std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  std::ostringstream oss;
  char tmc[30];
  std::strftime(tmc, 30, "%Y/%m/%d %T", gmtm);
  oss << tmc;
  oss.put(0);
  logFile << "[" << oss.str() << "|" << mseconds << "]  " << content;
  logFile.flush();
  lockLog.unlock();
}
