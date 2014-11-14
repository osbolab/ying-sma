#include <sma/app/contentdirectory.hpp>
#include <sma/app/sortdirectorybyrank.hpp>
#include <iostream>
#include <sma/app/contentdescription.hpp>
#include <string>

using namespace std;

int main ()
{

  ContentDirectory directory;
  // create content description 1

  ContentDescriptor file1("file1");
  file1.addNewChunk(0, "ajfdklajfklaj");
  file1.addNewChunk(1, "dkjfaklfjalja");
  file1.addNewChunk(2, "dakjfieijfifa");
  file1.addAttribute(ContentAttribute::PublishTime, "2014/10/14 13:30:11");  // use posix time or other more formal one later
  file1.addAttribute(ContentAttribute::Rank,"234");
  directory.addContentToDirectory(file1);
  
  //create content description 2
  ContentDescriptor file2("file2");
  file2.addNewChunk(0, "fajfijfiejfeajf");
  file2.addNewChunk(1, "fjakfjiejfiejfi");
  file2.addNewChunk(2, "dajiafijeufebub");
  file2.addNewChunk(3, "jihuhfafaffafjk");
  file2.addAttribute(ContentAttribute::PublishTime, "2014/10/14 13:30:15");
  directory.addContentToDirectory(file2);

  //create content description 3
  ContentDescriptor file3("file3");
  file3.addNewChunk(0, "akjfiajfiajfaaf");
  file3.addNewChunk(1, "ajfiajifjafnjdn");
  file3.addNewChunk(2, "jaifjiajfanfjah");
  file3.addNewChunk(3, "dfjaifjiasfakaf");
  file3.addNewChunk(4, "fdjakjfiafijafi");
  file3.addAttribute(ContentAttribute::PublishTime, "2014/10/13 13:30:11");
  directory.addContentToDirectory(file3);

  //create content description 4
  ContentDescriptor file4("file4");
  file4.addNewChunk(0, "jijinjajfakf");
  file4.addAttribute(ContentAttribute::Rank, "1");
  file4.addAttribute(ContentAttribute::Category, "Comedy");
  file4.addAttribute(ContentAttribute::PublishTime, "2014/10/16 13:30:11");
  directory.addContentToDirectory(file4);
  
  //create content description 5
  ContentDescriptor file5("file5");
  file5.addNewChunk(0, "jakfjakjfakf");
  file5.addNewChunk(1, "ajfiajiajfaa");
  file5.addAttribute(ContentAttribute::Rank, "400");
  file5.addAttribute(ContentAttribute::PublishTime, "2014/10/20 13:30:11");
  directory.addContentToDirectory(file5);

  // print directory without ranking, should be in order
  vector<ContentDescriptor> withoutRankList = directory.getNDirectory(5);
  vector<ContentDescriptor>::iterator iter = withoutRankList.begin();
  while (iter != withoutRankList.end())
  {
    iter->print();
    iter++;
  }
 
  // print directory after ranking, should be 4->1->5->2->3 
  directory.rankDirectory();
  vector<ContentDescriptor> afterRankList = directory.getNDirectory(5);
  vector<ContentDescriptor>::iterator iter2 = afterRankList.begin();
  while (iter2 != afterRankList.end())
  {
    iter2->print();
    iter2++;
  }
  return 0;
}
