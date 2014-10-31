#include <iostream>
#include "DataLayer.hh"
#include <cassert>

using namespace std;

int main ()
{
  DataLayer datalayer;
  string chunkID = "f2809dc1e3a7544043b8aafbd5011b0b";
 
  assert (datalayer.hasChunk(chunkID) == false);
  datalayer.storeChunk("./media/test_chunk", false);
  assert (datalayer.hasChunk(chunkID) == true);
  ifstream fetchFileStream;
  datalayer.fetchChunk(chunkID, fetchFileStream);
  assert(fetchFileStream.is_open());
  ifstream::pos_type nSize = fetchFileStream.tellg();
  cout << "File size is " << nSize << " bytes." << endl;
  ofstream testfout("./media/copy_chunk");
  fetchFileStream.seekg(0, std::ios::beg);
  istreambuf_iterator<char> begin_source(fetchFileStream);
  istreambuf_iterator<char> end_source;
  ostreambuf_iterator<char> begin_dest(testfout);
  copy(begin_source, end_source, begin_dest);
  fetchFileStream.close();
  testfout.close();
 // datalayer.deleteChunk(chunkID);
 // assert(datalayer.hasChunk(chunkID) == false); 
  return 0;
}
