/**
* @version      DGEMV Parallel - GrPPI v0.3
* @copyright    Copyright (C) 2017 Universidad Carlos III de Madrid. All rights reserved.
* @license      GNU/GPL, see LICENSE.txt
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You have received a copy of the GNU General Public License in LICENSE.txt
* also available in <http://www.gnu.org/licenses/gpl.html>.
*
* See COPYRIGHT.txt for copyright notices and details.
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <numeric>
#include <random>
#include "grppi.h"
#include "dyn/dynamic_execution.h"

// ddot: res = row * vec';
double ddot(const std::vector<double>& row, 
  const std::vector<double>& vec,
  const grppi::dynamic_execution& exec)
{
  // ****** GRPPI code must be placed from here ***** //
  double res= 0.0;
  for (int i = 0; i < row.size(); i++)
    res += row[i] * vec[i];
  return res;
  // ****** to here ***** //
}

// dgemv: res = mat * vec;
void dgemv(const std::vector<std::vector<double>>& mat, 
  const std::vector<double>& vec,
  std::vector<double>& res,
  const grppi::dynamic_execution& exec)
{  
  // ****** GRPPI code must be placed from here ***** //
  for (int i = 0; i < mat.size(); i++)
    res[i] = ddot(mat[i], vec, exec);
  // ****** to here ***** //
}

grppi::dynamic_execution execution_mode(const std::string & opt, int nr_threads) 
{
  using namespace grppi;
  if ("seq" == opt) return sequential_execution{};
  if ("thr" == opt) return parallel_execution_native{nr_threads};
  if ("omp" == opt) return parallel_execution_omp{nr_threads};
  if ("tbb" == opt) return parallel_execution_tbb{nr_threads};
  return {};
}

void generate(std::vector<std::vector<double>>& mat,
  std::vector<double>& vec)
{
  std::random_device rdev;
  std::uniform_int_distribution<> gen{1,1000};

  for (int i= 0; i < mat.size(); i++)
    for (int j= 0; j < mat[0].size(); j++)
      mat[i][j]= gen(rdev);

  for (int i= 0; i < vec.size(); i++)
    vec[i]= gen(rdev);
}

int main(int argc, char *argv[])
{
  // parameters checking
  if (argc != 5){
    std::cout << "Usage: " << argv[0]
              << " rows cols mode nr_threads" << std::endl;
    return -1;
  }

  int rows = std::stoi(argv[1]),
      cols = std::stoi(argv[2]);
  auto exec = execution_mode(argv[3], std::stoi(argv[4]));      

  std::vector<std::vector<double>> 
    mat(rows, std::vector<double>(cols));
  std::vector<double> vec(cols);
  std::vector<double> res(rows);

  generate(mat, vec);
  
  std::chrono::time_point<std::chrono::system_clock> start, end; 
  start = std::chrono::system_clock::now();
  dgemv(mat, vec, res, exec);
  end = std::chrono::system_clock::now();

  // print preformance results
  int elapsed_seconds = std::chrono::duration_cast
    <std::chrono::milliseconds>(end-start).count();

  std::cout << "Execution time: " << elapsed_seconds << " milliseconds" << std::endl;

  return 0;
}
