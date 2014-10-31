#include "md5.hh"
#include <iostream>


int main (int argc, char* argv[])
{

  MD5 md5;
  std::cout << "MD5 of file " << argv[1] << " is " << md5.digestFile(argv[1]) << std::endl;
  return 0;

}
