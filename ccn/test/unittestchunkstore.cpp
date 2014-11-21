//this file is outdated. don't use.

#include <iostream>
#include <sma/app/chunkstore.hpp>
#include <sma/app/plainchunkstore.hpp>
#include <cassert>
#include <fstream>

using namespace std;

int main ()
{
  ChunkStore* contentStorePtr = new PlainChunkStore("./cache/");
  contentStorePtr->storeChunk("./chunks/kmd-mdjnka.avi.part1", false); //chunkID
  assert(contentStorePtr->hasChunk("2083d34f3bc789675d16138abd21c667"));
  ifstream fetchFileStream;
  contentStorePtr->fetchChunk("2083d34f3bc789675d16138abd21c667", fetchFileStream);
  assert(fetchFileStream.is_open());
  ifstream::pos_type nSize = fetchFileStream.tellg();
  cout << "File size is " << nSize << " bytes." << endl;
  char* memBlock = new char[nSize];
  ofstream testfout ("output.out");
  testfout.write(memBlock, nSize);
  testfout.close();
  fetchFileStream.close();    
  contentStorePtr->deleteChunk("2083d34f3bc789675d16138abd21c667");
  assert(contentStorePtr->hasChunk("2083d34f3bc789675d16138abd21c667") == false);
  delete contentStorePtr;
  
  return 0;
}
