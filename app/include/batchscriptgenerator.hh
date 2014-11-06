#ifndef BATCH_SCRIPT_GENERATOR_H
#define BATCH_SCRIPT_GENERATOR_H

#include <string>
#include <fstream>

class BatchScriptGenerator
{
public:
  BatchScriptGenerator (std::string fileName = DEFAULT_SCRIPT_FILE_NAME);
  void getScriptFin (std::ifstream& file) const;
private:
  void generateTopology();
  void generateActivity();
  double randomInRange (double min, double max);
  double randomLatitude();
  double randomLongitude();

  std::string scriptFileName;
  static double DEFAULT_WEST; // = 33.625;
  static double DEFAULT_EAST; // 33.6701
  static double DEFAULT_NORTH; // -112.1853
  static double DEFAULT_SOUTH; // -112.23944
  static std::string DEFAULT_SCRIPT_FILE_NAME; //
  static int DEFAULT_NUM_OF_NODES; // 10000
};

#endif
