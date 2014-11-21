#include <sma/app/batchscriptgenerator.hpp>
#include <ctime>
#include <cassert>


double BatchScriptGenerator::DEFAULT_WEST = 33.625;
double BatchScriptGenerator::DEFAULT_EAST = 33.670;
double BatchScriptGenerator::DEFAULT_NORTH = -112.1853;
double BatchScriptGenerator::DEFAULT_SOUTH = -112.23944;
std::string BatchScriptGenerator::DEFAULT_SCRIPT_FILE_NAME =
    "./emulationInbatch.txt";
int BatchScriptGenerator::DEFAULT_NUM_OF_NODES = 1000;

BatchScriptGenerator::BatchScriptGenerator(std::string fileName)
  : scriptFileName(fileName)
{
  srand(time(nullptr));
  generateTopology();
  generateActivity();
}

void BatchScriptGenerator::generateActivity()
{
  std::ofstream fout(scriptFileName, std::ofstream::out | std::ofstream::app);
  fout << "publish 1 ./media/beautiful.mp3 test.mp3" << std::endl;
  fout.close();
}

void BatchScriptGenerator::generateTopology()
{
  std::ofstream fout(scriptFileName);
  fout << "create network adhoc" << std::endl;
  for (int i = 0; i < DEFAULT_NUM_OF_NODES; i++) {
    fout << "create device " << i << std::endl;
    fout << "setgps " << i << " " << randomLatitude() << " "
         << randomLongitude() << std::endl;
    fout << "connect " << i << std::endl;
  }
  fout.close();
}

double BatchScriptGenerator::randomLatitude()
{
  return randomInRange(DEFAULT_WEST, DEFAULT_EAST);
}

double BatchScriptGenerator::randomLongitude()
{
  return randomInRange(DEFAULT_SOUTH, DEFAULT_NORTH);
}

double BatchScriptGenerator::randomInRange(double min, double max)
{
  return (double) rand() / RAND_MAX * (max - min) + min;
}

void BatchScriptGenerator::getScriptFin(std::ifstream& file) const
{
  file.open(scriptFileName);
  assert(file.is_open());
}
