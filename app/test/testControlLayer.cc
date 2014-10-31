#include <iostream>
#include "ControlLayer.hh"
#include <vector>
#include <cassert>

using namespace std;

int main (int argc, char* argv[])
{
  ControlLayer control;
  assert(argc == 3);

  vector<pair<ContentAttribute::META_TYPE, std::string> > attri_pair;
  attri_pair.push_back(make_pair(ContentAttribute::Rank, "234"));
  attri_pair.push_back(make_pair(ContentAttribute::PublishTime, "12:00"));

  control.publishContent(argv[1], attri_pair);
  control.restoreContentAs(argv[1], argv[2]);
  return 0;
}
