#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
using namespace std;

int regroup( int nParts, char* filename);

int main (int argc, char * argv[])
{
  ostringstream sStringer;
  ifstream::pos_type nSize;
  ifstream fSource(argv[1], ios_base::ate|ios::binary|ios::in);
  char * sMemBlock;
  nSize = fSource.tellg();
  string sExtension = strstr(argv[1], ".");
  int nGetPointer = 0;
  string sChunkSize = argv[2];
  istringstream nIntegerer(sChunkSize);
  int nChunkSize;
  nIntegerer >> nChunkSize;
  int nLastChunkSize = nChunkSize;
  int nPartNumber = 1;
  string sDestinationFile;

  if (fSource.is_open())
  {
    cout << "File input : " << argv[1] << endl;
    cout << "Size: " << nSize << "bytes" << endl;
    cout << "Extension : " << sExtension << endl;

    fSource.seekg(0, ios::beg);

    bool isLastChunk = false;

    while (fSource)   // tweek
    {
      fSource.seekg(nGetPointer, ios::beg);
      if (nGetPointer + nChunkSize > nSize)
      {
        isLastChunk = true;
        nLastChunkSize = nSize % nChunkSize;
        sMemBlock = new char[nLastChunkSize];
        fSource.read(sMemBlock, nLastChunkSize);
      }
      else
      {
        sMemBlock = new char[nChunkSize];
        fSource.read(sMemBlock, nChunkSize);
      }

      sDestinationFile = argv[1];
      sDestinationFile.append(".part");
      sStringer.str("");
      sStringer << nPartNumber;
      sDestinationFile.append(sStringer.str());

      cout << "Destination file: " << sDestinationFile << endl;
      ofstream fDestination(sDestinationFile.c_str());
      int chunkSize = isLastChunk ? nLastChunkSize : nChunkSize;
      fDestination.write(sMemBlock, chunkSize);
      fDestination.close();  // add temporarily
      nGetPointer += chunkSize;
      nPartNumber += 1;
      assert(nPartNumber < 2000);
      delete[] sMemBlock;
      sMemBlock = NULL;
    }
    fSource.close();
    regroup(nPartNumber-1, argv[1]);
  }
  return 0;
}

int regroup(int nParts, char * filename)
{
  cout << "regrouping..." << endl;
  string sChunkFile;
  ostringstream sStringer;
  int nPartNumber = 1;
  char * sMemBlock;
  ifstream::pos_type nSize;
 
  string newFileName = "new";
  newFileName.append(filename);
  ofstream fRetour(newFileName);
 
  for (nPartNumber = 1; nPartNumber <= nParts; nPartNumber++)
  {
    sChunkFile = filename;
    sChunkFile.append(".part");
    sStringer.str("");
    sStringer << nPartNumber;
    sChunkFile.append(sStringer.str());
    ifstream fChunk(sChunkFile.c_str(), ios::in|ios::binary|ios::ate);
    nSize = fChunk.tellg();
    sMemBlock = new char[nSize];
    fChunk.seekg(0, ios::beg);
    fChunk.read(sMemBlock, nSize);
    fChunk.close();
    fRetour.write(sMemBlock, nSize);
    delete[] sMemBlock;
    sMemBlock = NULL
  }
  fRetour.close();
  return 0;
}
