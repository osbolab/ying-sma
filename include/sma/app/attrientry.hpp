#pragma once


enum META_TYPE {PublishTime, Category, Location};

struct AttriEntry
{
  META_TYPE type;
  std::string typeValue;

  AttriEntry(META_TYPE newType, std::string newTypeValue):type(newType), typeValue(newTypeValue) {}
};


