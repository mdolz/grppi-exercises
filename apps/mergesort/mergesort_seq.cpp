/**
* @version      Mergesort Sequential - GrPPI v0.3
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

#include <vector>
#include <iostream>
#include <stdlib.h>
#include <numeric>
#include <stdexcept>
#include <random>
#include <chrono>

struct range {
  std::vector<int>::iterator first, last;
  auto size() const { return distance(first,last); }
};

std::vector<range> divide(range r);
std::vector<int> merge(std::vector<int>& first, std::vector<int>& second);

// Recursive merge sort function
std::vector<int> merge_sort_(range sequence)
{ 
  // Solver
  if(sequence.size() <= 1 ) 
    return std::vector<int>(sequence.first, sequence.last);
  // Divider
  auto r = divide(sequence);
  // Combiner
  auto first= merge_sort_(r[0]);
  auto second= merge_sort_(r[1]);
  return merge(first,second);
}

// Initial merge sort function
std::vector<int> merge_sort(std::vector<int> sequence)
{
  return merge_sort_(range{sequence.begin(), sequence.end()});
}

// Divides a range in two
std::vector<range> divide(range r) 
{
  auto mid = r.first + distance(r.first, r.last) / 2;
  return { {r.first,mid} , {mid, r.last} };
}

// Merges and sorts two vectors
std::vector<int> merge(std::vector<int>& first, std::vector<int>& second)
{
  std::vector<int> result;
  int ifirst= 0, isecond= 0;
  while (ifirst < first.size() && isecond < second.size()) {
    if (first[ifirst] < second[isecond]) {
      result.push_back(first[ifirst]);
      ifirst++;
    } 
    else {
      result.push_back(second[isecond]);
      isecond++;
    }
  }
  result.insert(result.end(), first.begin()+ifirst, first.end());
  result.insert(result.end(), second.begin()+isecond, second.end());
  return result;
}

std::vector<int> generate_sequence(int size)
{
  std::random_device rdev;
  std::uniform_int_distribution<> gen{1,1000};

  std::vector<int> v;
  for (int i=0; i<size; ++i) {
    v.push_back(gen(rdev));
  }
  return v;
}

void print_sequence(std::vector<int> sequence)
{
  for(auto i : sequence)
    std::cout << i << " ";
  std::cout<<std::endl;
}

int main(int argc, char *argv[])
{
  // parameters checking
  if(argc != 3){
    std::cout << "Usage: " << argv[0]
              << " vector_size output" << std::endl;   
    return -1;
  }
  auto size = std::stoi(argv[1]);
  std::string output = argv[2];
  auto sequence = generate_sequence(size);

  if (output == "yes"){
    std::cout << "Original sequence: "; 
    print_sequence(sequence); 
  }

  auto start = std::chrono::high_resolution_clock::now();
  auto sorted_sequence = merge_sort(sequence);
  auto end = std::chrono::high_resolution_clock::now();

  if (output == "yes"){
    std::cout<<"Result sequence: "; 
    print_sequence(sorted_sequence); 
  }

  // print preformance results
  int elapsed_seconds =
    std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

  std::cout << "Execution time: " << elapsed_seconds << " milliseconds" << std::endl;

  return 0;
}
