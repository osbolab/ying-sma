#pragma once


#include <map>
#include <string>
#include <cstring>
#include <vector>
#include <ctime>


namespace ContentAttribute
{
enum META_TYPE {
  PublishTime = 0,
  Category = 1,
  Location = 2,
  Rank = 3,
  Invalid = 4
};
static const char* META_TYPE_STR[] = {
    "PublishTime", "Category", "Location", "Rank"};
inline META_TYPE to_META_TYPE(const char* str)
{
  if (std::strcmp(str, "PublishTime") == 0)
    return PublishTime;
  else if (std::strcmp(str, "Category") == 0)
    return Category;
  else if (std::strcmp(str, "Location") == 0)
    return Location;
  else if (std::strcmp(str, "Rank") == 0)
    return Rank;
  else
    return Invalid;
}
}

class ContentDescriptor
{
public:
  ContentDescriptor(std::string filename);
  void addNewChunk(unsigned int seqNo,
                   std::string chunkID);    // only owner can do that
  void delChunk(unsigned int seqNo);        // only owner can do that
  void addAttribute(ContentAttribute::META_TYPE type,
                    std::string typeValue);    // only owner can do that
  void
  delAttribute(ContentAttribute::META_TYPE type);    // only owner can do that
  std::string getContentName() const;
  void setContentName(std::string filename);
  std::string getChunkID(unsigned int seqNo) const;
  std::string getTypeValue(ContentAttribute::META_TYPE type) const;
  void print() const;    // for unit test, should be exported in JSON format for
                         // network use
  std::vector<std::string> getChunkList() const;
  std::map<ContentAttribute::META_TYPE, std::string> getMetaPairList() const;
  bool newerThan(const ContentDescriptor& descriptor) const;
  std::time_t timestampToTimeT() const;
  static time_t timestampToTime(std::string timestamp);

private:
  std::string contentName;
  std::map<unsigned int, std::string> chunkList;
  std::map<ContentAttribute::META_TYPE, std::string> attriList;
};


