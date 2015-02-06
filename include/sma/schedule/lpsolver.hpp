#pragma once

#include <glpk.h>
#include <vector>
#include <sstream>
#include <cassert>
#include <iostream>

class LPSolver
{
public:
  static void solve ( int max_ttl,
                      int num_of_blocks,
                      int storage,
                      int bandwidth,
                      int num_of_neighbor,
                      const std::vector<std::vector<double> >&  utility)
  {
    glp_prob *mip = glp_create_prob();
    glp_set_prob_name(mip, "joint_scheduling");
    glp_set_obj_dir(mip, GLP_MAX);

    //// row

    int num_of_rows = (max_ttl+2) 
                    + (max_ttl+1) 
                    + num_of_blocks 
                    + num_of_blocks * (max_ttl+1);

    glp_add_rows(mip, num_of_rows);

    int row_index = 1;

    // constraint set 1
    for (int i=0; i<=max_ttl+1; i++)
    {
      std::ostringstream constraint_name;
      constraint_name << "c" << row_index;
      glp_set_row_name(mip, row_index, constraint_name.str().c_str());
      glp_set_row_bnds(mip, row_index, GLP_UP, 0.0, storage);
      row_index++;
    }

    // constraint set 2

    for (int i=0; i<=max_ttl; i++)
    {
      std::ostringstream constraint_name;
      constraint_name << "c" << row_index;
      glp_set_row_name (mip, row_index, constraint_name.str().c_str());
      glp_set_row_bnds (mip, row_index, GLP_UP, 0.0, bandwidth); 
      row_index++;
    }

    // constraint set 3
    for (int i=0; i<num_of_blocks; i++)
    {
      std::ostringstream constraint_name;
      constraint_name << "c" << row_index;
      glp_set_row_name (mip, row_index, constraint_name.str().c_str());
      glp_set_row_bnds (mip, row_index, GLP_FX, 0, 0); // 1->0
      row_index++;
    }

    //constraint set 4

    for (int i=0; i<num_of_blocks; i++)
      for (int j=0; j<=max_ttl; j++)
      {
        std::ostringstream constraint_name;
        constraint_name << "c" << row_index;
        glp_set_row_name (mip, row_index, constraint_name.str().c_str());
        glp_set_row_bnds (mip, row_index, GLP_UP, 0.0, 0.0);
        row_index++;
      }

    //// column
    
    int num_of_cols = (max_ttl+2) * num_of_blocks;
    glp_add_cols(mip, num_of_cols);
    
    int col_index = 1;

    for (int i=0; i<num_of_blocks; i++)
      for (int j=0; j<=max_ttl+1; j++)
      {
        std::ostringstream constraint_name;
        constraint_name << "x" << col_index;
        glp_set_col_name (mip, col_index, constraint_name.str().c_str());
        glp_set_col_bnds (mip, col_index, GLP_DB, 0, 1);
        if (j==0)
        {
          double total_utilities = 0;
          for (int k=0; k<num_of_neighbor; k++)
          {
            total_utilities += utility[k][i];
          }
          glp_set_obj_coef (mip, col_index, total_utilities);
        }
        else
        {
          glp_set_obj_coef (mip, col_index, 0);
        }
        glp_set_col_kind (mip, col_index, GLP_BV); //binary
        col_index++;
      }


    //// assign coef at the constarint matrix

    std::vector<int> ia_vec;
    std::vector<int> ja_vec;
    std::vector<int> arr_vec;

    // construct coef array for constraint set 1

    for (int t=0; t<= max_ttl+1; t++)
      for (int c=0; c<num_of_blocks; c++)
      {
        ia_vec.push_back (1+t);
        ja_vec.push_back (1 + c * (max_ttl+2) + t);
        arr_vec.push_back(1);
      }

    // construct coef array for constraint set 2

    for (int t=0; t<=max_ttl; t++)
      for (int c=0; c<num_of_blocks; c++)
      {
        ia_vec.push_back (max_ttl + 3 + t);
        ia_vec.push_back (max_ttl + 3 + t);
        ja_vec.push_back (1 + c * (max_ttl+2) + t);
        ja_vec.push_back (1 + c * (max_ttl+2) + t + 1);
        arr_vec.push_back (1);
        arr_vec.push_back (-1);
      }

    
    // construct coef array for constraint set 3

    for (int c=0; c<num_of_blocks; c++)
    {
//      ia_vec.push_back (2 * max_ttl + 4 + c);
      ia_vec.push_back (2 * max_ttl + 4 + c);
//      ja_vec.push_back (1 + c * (max_ttl+2));
      ja_vec.push_back (1 + c * (max_ttl+2) + max_ttl + 1);
 //     arr_vec.push_back(1);
      arr_vec.push_back(1);
    }


    // construct coef array for constraint set 4
    
    for (int c=0; c<num_of_blocks; c++)
      for (int t=0; t<=max_ttl; t++)
      {
        ia_vec.push_back(2 * max_ttl + 4 + num_of_blocks
                + c * (max_ttl+1) + t);
        ia_vec.push_back(2 * max_ttl + 4 + num_of_blocks
                + c * (max_ttl+1) + t);
        ja_vec.push_back(1 + c * (max_ttl+2) + t);
        ja_vec.push_back(1 + c * (max_ttl+2) + t + 1);
        arr_vec.push_back(-1);
        arr_vec.push_back(1);
      }

    assert (ia_vec.size() == ja_vec.size() && ia_vec.size() == arr_vec.size());

    int sz = ia_vec.size();
    std::cout <<"size is " << sz << std::endl;

    int * ia = new int[sz+1];
    ia[0] = 0;
    int * ja = new int[sz+1];
    ja[0] = 0;
    double * arr = new double[sz+1];
    arr[0] = 0;

    std::copy (ia_vec.begin(), ia_vec.end(), ia+1);
    std::copy (ja_vec.begin(), ja_vec.end(), ja+1);
    std::copy (arr_vec.begin(), arr_vec.end(), arr+1);

    for (int i=0; i<=sz; i++)
    {
      std::cout << "ia = " << ia[i] << ", " << "ja = " << ja[i] 
          << ", arr = " << arr[i] << std::endl; 
    }

    glp_load_matrix (mip, sz, ia, ja, arr);

    glp_iocp parm;
    glp_init_iocp(&parm);
    parm.presolve = GLP_ON;

    int err = glp_intopt(mip, &parm);

    double max_utility = glp_mip_obj_val (mip);
    
    std::cout << "max utility is " << max_utility << std::endl;
    
    std::vector<double> result;

    for (int i=0; i<num_of_cols; i++)
    {
      std::cout << "retrieving " << i << "..." << std::endl;
      result.push_back (glp_mip_col_val(mip, i+1)); 
    }

    delete[] ia;
    delete[] ja;
    delete[] arr;

    for (int it=0; it<num_of_cols; it++)
    {
      int c = it / (max_ttl+2);
      int t = it % (max_ttl+2);
      std::cout << "content "<< c 
          <<" at " << t << " = " << result[it] << std::endl;
    }
  }
};
