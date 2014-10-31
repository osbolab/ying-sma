#include <iostream>
#include "control/Segmenter.hh"
#include "data/ChunkStore.hh"
#include "data/PlainChunkStore.hh"

using namespace std;

int main (int argc, char* argv[])
{
//  ChunkStore* contentStorePtr = new PlainChunkStore("./build/cache/");  
  DataLayer datalayer;
  Segmenter segObj;
  vector<ChunkID> chunkIDs;
  segObj.storeFile(argv[1], chunkIDs, datalayer);
  vector<ChunkID>::iterator iter = chunkIDs.begin();
  cout << "The chunk id list of file " << argv[1] << " is: " << endl;
  while (iter != chunkIDs.end())
  {
    cout <<*iter << '\t';
    iter++; 
  }

  // re-group the chunks
  
  segObj.loadFileFromChunks (argv[2], chunkIDs, datalayer);
}