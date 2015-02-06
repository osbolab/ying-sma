#include "lpsolver.hpp"
#include <vector>

int main ()
{

  int max_ttl = 10;
  int num_of_blocks = 30;
  int storage = 20;
  int bandwidth = 20;
  int num_of_neighbors = 10;
  double utility_array[][3] = 
  {
    {1, 0, 0},
    {0, 0.5, 0.5},
    {0.7, 0.3, 0},
    {0.5, 0, 0.5}
  };

  std::vector<std::vector<double> > utility(num_of_neighbors, 
                                           std::vector<double>(num_of_blocks));
  for (int i = 0; i < num_of_neighbors; ++i)
  {
    utility[i].assign(utility_array[i], utility_array[i] + num_of_blocks);
  } 

  LPSolver::solve (max_ttl,
                  num_of_blocks,
                  storage,
                  bandwidth,
                  num_of_neighbors,
                  utility);
}
