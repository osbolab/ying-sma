#include "control/Segmenter.hh"
#include "data/ChunkStore.hh"
#include "data/PlainChunkStore.hh"

#include "gtest/gtest.h"

#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <string>


TEST(Segmenter, store_and_load)
{
  std::string path = "/tmp/" + std::rand() + std::rand();

  char buf[1024 * 1024];
  for (std::size_t i = 0; i < sizeof(buf); ++i) {
    buf[i] = static_cast<char>(std::rand());
  }
  FILE* temp_file = fopen(path.c_str(), "wb");
  fwrite(buf, sizeof(char), sizeof(buf), temp_file);
  fseek(temp_file, 0, SEEK_SET);

  //  ChunkStore* contentStorePtr = new PlainChunkStore("./build/cache/");
  DataLayer datalayer("/tmp/");
  Segmenter segObj;
  std::vector<ChunkID> chunkIDs;
  segObj.storeFile(path.c_str(), chunkIDs, datalayer);
  std::vector<ChunkID>::iterator iter = chunkIDs.begin();
  std::cout << "The chunk id list of file " << path << " is: " << std::endl;
  while (iter != chunkIDs.end()) {
    std::cout << *iter << '\t';
    iter++;
  }

  // re-group the chunks
  segObj.loadFileFromChunks("/tmp/", chunkIDs, datalayer);

  fclose(temp_file);
}
