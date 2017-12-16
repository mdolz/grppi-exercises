/**
* @version      Moving Average Sequential - GrPPI v0.3
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
#include <chrono>
#include <numeric>
#include <vector>

int read_sensor();

void moving_average(int win_size,
  int slide, int num_items)
{
  long long n=0;
  std::vector<int> buffer;
  bool finish= false;

  while (!finish) {
    // slide window
    if (buffer.size() >= slide)
      buffer.erase(buffer.begin(), buffer.begin() + slide);
    // complete a window
    while (buffer.size() < win_size) {
      buffer.push_back(read_sensor());
      if (n++ > num_items) { 
        finish= true; 
        break; 
      }
    }
    // compute average
    float avg= std::accumulate(buffer.begin(), buffer.end(), 0) / 
      (float) win_size;
    // print values
    std::cout << avg << std::endl;
  }
}

int read_sensor()
{ 
  static long i = 0;
  return i++; // rand() % 2000;
}

int main(int argc, char *argv[]) 
{
  // parameters checking
  if(argc != 4){
    std::cout << "Usage: " << argv[0] 
              << " window_size slide num_items" << std::endl;
    return -1;
  } 

  auto win_size = std::stoi(argv[1]);
  auto slide = std::stoi(argv[2]);
  auto num_items = std::stol(argv[3]);
  
  auto start = std::chrono::high_resolution_clock::now();
  moving_average(win_size, slide, num_items);
  auto end = std::chrono::high_resolution_clock::now();

  // print preformance results
  int elapsed_seconds =
    std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

  std::cout << "Execution time: " << elapsed_seconds << " milliseconds" << std::endl;

  return 0;
}