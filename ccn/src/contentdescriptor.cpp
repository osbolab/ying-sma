#include <sma/app/contentdescriptor.hpp>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <cassert>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>


ContentDescriptor::ContentDescriptor(std::string filename)
  : contentName(filename)
{
}

void ContentDescriptor::addNewChunk(unsigned int seqNo, std::string chunkID)
{
  chunkList.insert(make_pair(seqNo, chunkID));
}

void ContentDescriptor::addAttribute(ContentAttribute::META_TYPE type,
                                      std::string typeValue)
{
  attriList.insert(make_pair(type, typeValue));
}

void ContentDescriptor::delChunk(unsigned int seqNo)
{
  std::map<unsigned int, std::string>::iterator iter = chunkList.find(seqNo);
  if (iter != chunkList.end())
    chunkList.erase(iter);
}

void ContentDescriptor::delAttribute(ContentAttribute::META_TYPE type)
{
  std::map<ContentAttribute::META_TYPE, std::string>::iterator iter =
      attriList.find(type);
  if (iter != attriList.end())
    attriList.erase(iter);
}

std::string ContentDescriptor::getContentName() const
{
  return contentName;
}

void ContentDescriptor::setContentName(std::string filename)
{
  this->contentName = filename;
}

std::string ContentDescriptor::getChunkID(unsigned int seqNo) const
{
  std::map<unsigned int, std::string>::const_iterator iter =
      chunkList.find(seqNo);
  return ((iter == chunkList.end()) ? "" : iter->second);
}

std::string
ContentDescriptor::getTypeValue(ContentAttribute::META_TYPE type) const
{
  std::map<ContentAttribute::META_TYPE, std::string>::const_iterator iter =
      attriList.find(type);
  return ((iter == attriList.end()) ? "" : iter->second);
}

void ContentDescriptor::print() const
{
  std::cout << "ContentName: " << this->contentName << std::endl;
  std::map<unsigned int, std::string>::const_iterator chunk_iter =
      chunkList.begin();
  while (chunk_iter != chunkList.end()) {
    std::cout << '\t' << chunk_iter->first << " : " << chunk_iter->second
              << std::endl;
    chunk_iter++;
  }
  std::map<ContentAttribute::META_TYPE, std::string>::const_iterator
      attri_iter = attriList.begin();
  while (attri_iter != attriList.end()) {
    ContentAttribute::META_TYPE enumValue = attri_iter->first;
    std::cout << '\t' << ContentAttribute::META_TYPE_STR[enumValue] << " : "
              << attri_iter->second << std::endl;
    attri_iter++;
  }
  std::cout << std::endl;
}

std::vector<std::string> ContentDescriptor::getChunkList() const
{
  std::vector<std::string> result;
  int size = chunkList.size();
  for (int i = 0; i < size; i++) {
    std::map<unsigned int, std::string>::const_iterator iter =
        chunkList.find(i);
    assert(iter != chunkList.end());
    result.push_back(iter->second);
  }
  return result;
}

std::map<ContentAttribute::META_TYPE, std::string>
ContentDescriptor::getMetaPairList() const
{
  return attriList;
}

bool ContentDescriptor::newerThan(const ContentDescriptor& description) const
{
  // must make sure the timestamp field always exists when creating the meta
  // data.
  //  std::string time1Str = this->getTypeValue(ContentAttribute::PublishTime);
  //  std::string time2Str =
  //  description.getTypeValue(ContentAttribute::PublishTime);
  //  std::cout << "compare time " << time1Str
  //            << " and " << time2Str << std::endl;
  time_t time1 = this->timestampToTimeT();
  time_t time2 = description.timestampToTimeT();
  //  std::cout << "time_t: " << time1 << " v.s. " << time2 << std::endl;
  //  std::cout << std::difftime(time1, time2) << std::endl;
  return std::difftime(time1, time2) > 0.0;
}

std::time_t ContentDescriptor::ContentDescriptor::timestampToTimeT() const
{
  std::string timeStampStr = this->getTypeValue(ContentAttribute::PublishTime);
  std::istringstream iss(timeStampStr);
  std::tm when;
  char tmc[30];
  std::memset(tmc, 0, 30);
  for (std::size_t i; i < 29; ++i) {
    char c;
    if ((c = iss.get()) == 0)
      break;
    tmc[i] = c;
  }
  ::strptime(tmc, "%Y/%m/%d %T", &when);    // format depends on the locale
  //  std::cout << std::put_time(&when, "%a %h %d %H:%M:%S %G") << std::endl;
  //  std::cout << std::put_time(&when, "%Y/%m/%d %T") << std::endl;
  std::time_t time = timegm(&when);
  return time;
}
