#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <cassert>


int regroup(int nParts, char* filename);

int main(int argc, char* argv[])
{
  std::ostringstream sstringer;
  std::ifstream::pos_type nSize;
  std::ifstream fSource(argv[1],
                        std::ios_base::ate | std::ios::binary | std::ios::in);
  char* sMemBlock;
  nSize = fSource.tellg();
  std::string sExtension = std::strstr(argv[1], ".");
  int nGetPointer = 0;
  std::string sChunkSize = argv[2];
  std::istringstream nIntegerer(sChunkSize);
  int nChunkSize;
  nIntegerer >> nChunkSize;
  int nLastChunkSize = nChunkSize;
  int nPartNumber = 1;
  std::string sDestinationFile;

  if (fSource.is_open()) {
    std::cout << "File input : " << argv[1] << std::endl;
    std::cout << "Size: " << nSize << "bytes" << std::endl;
    std::cout << "Extension : " << sExtension << std::endl;

    fSource.seekg(0, std::ios::beg);

    bool isLastChunk = false;

    while (fSource)    // tweek
    {
      fSource.seekg(nGetPointer, std::ios::beg);
      if (nGetPointer + nChunkSize > nSize) {
        isLastChunk = true;
        nLastChunkSize = nSize % nChunkSize;
        sMemBlock = new char[nLastChunkSize];
        fSource.read(sMemBlock, nLastChunkSize);
      } else {
        sMemBlock = new char[nChunkSize];
        fSource.read(sMemBlock, nChunkSize);
      }

      sDestinationFile = argv[1];
      sDestinationFile.append(".part");
      sstringer.str("");
      sstringer << nPartNumber;
      sDestinationFile.append(sstringer.str());

      std::cout << "Destination file: " << sDestinationFile << std::endl;
      std::ofstream fDestination(sDestinationFile.c_str());
      int chunkSize = isLastChunk ? nLastChunkSize : nChunkSize;
      fDestination.write(sMemBlock, chunkSize);
      fDestination.close();    // add temporarily
      nGetPointer += chunkSize;
      nPartNumber += 1;
      assert(nPartNumber < 2000);
      delete[] sMemBlock;
      sMemBlock = nullptr;
    }
    fSource.close();
    regroup(nPartNumber - 1, argv[1]);
  }
  return 0;
}

int regroup(int nParts, char* filename)
{
  std::cout << "regrouping..." << std::endl;
  std::string sChunkFile;
  std::ostringstream sstringer;
  int nPartNumber = 1;
  char* sMemBlock;
  std::ifstream::pos_type nSize;

  std::string newFileName = "new";
  newFileName.append(filename);
  std::ofstream fRetour(newFileName);

  for (nPartNumber = 1; nPartNumber <= nParts; nPartNumber++) {
    sChunkFile = filename;
    sChunkFile.append(".part");
    sstringer.str("");
    sstringer << nPartNumber;
    sChunkFile.append(sstringer.str());
    std::ifstream fChunk(sChunkFile.c_str(),
                         std::ios::in | std::ios::binary | std::ios::ate);
    nSize = fChunk.tellg();
    sMemBlock = new char[nSize];
    fChunk.seekg(0, std::ios::beg);
    fChunk.read(sMemBlock, nSize);
    fChunk.close();
    fRetour.write(sMemBlock, nSize);
    delete[] sMemBlock;
    sMemBlock = nullptr;
  }
  fRetour.close();
  return 0;
}
