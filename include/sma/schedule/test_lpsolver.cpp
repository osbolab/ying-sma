#include "lpsolver.hpp"
#include <vector>
#include <cstddef>

int main ()
{

  std::size_t max_ttl = 10;
  std::size_t num_of_blocks = 3;
  std::size_t storage = 20;
  std::size_t bandwidth = 1;
  std::size_t num_of_neighbors = 4;

//  int deadline[] = {10, 10, 1};
  int deadline_array[][3] =
  {
    {0, -1, -1},
    {-1, 0, 10},
    {0, 0, -1},
    {0, -1, 2},
  };

  float utility_array[][3] = 
  {
    {1, 0, 0},
    {0, 0.5, 0.5},
    {0.7, 0.3, 0},
    {0.5, 0, 0.5}
  };

  std::vector<std::vector<float> > utility(num_of_neighbors, 
                                           std::vector<float>(num_of_blocks));

  std::vector<std::vector<int> > deadline(num_of_neighbors,
                                         std::vector<int>(num_of_blocks));

  for (std::size_t i = 0; i < num_of_neighbors; ++i)
  {
    utility[i].assign(utility_array[i], utility_array[i] + num_of_blocks);
    deadline[i].assign(deadline_array[i], deadline_array[i] + num_of_blocks);
  } 

  std::vector<std::vector<std::size_t> > sched_result (num_of_blocks,
          std::vector<std::size_t>(max_ttl+2));

  LPSolver::solve (max_ttl,
                  num_of_blocks,
                  storage,
                  bandwidth,
                  num_of_neighbors,
                  deadline,
                  utility,
                  sched_result);

  std::size_t num_of_t = sched_result[0].size();
  std::size_t num_of_c = sched_result.size();

  for (std::size_t c=0; c<num_of_c; c++)
    if (sched_result[c][0] == 1)
    {
        std::cout << "freeze block " << c << std::endl;
        std::size_t t = 1;
        for (; t<num_of_t; t++)
        {
          if (sched_result[c][t] == 0)
          {
            std::cout << "broadcast block " << c 
                      << " at time " << t-1 << std::endl;
            break;
          }
        }
        if (t == num_of_t)
          std::cout << "block " << c << " not handled!" << std::endl;
    }
    else
       std::cout << "unfreeze block " << c << std::endl;
}
