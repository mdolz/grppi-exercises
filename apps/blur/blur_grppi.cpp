/**
* @version      Blur Filter Parallel - GrPPI v0.3
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
#include "grppi.h"
#include "dyn/dynamic_execution.h"

auto blur(std::vector<unsigned char> frame, int frame_cols,
          std::vector<int> kernel, int kernel_cols,
          const grppi::dynamic_execution& exec)
{
  std::vector<unsigned char> result(frame.size(),0);
  std::vector<int> offset;

  for(int k = 0; k < kernel.size(); k++)
    offset.push_back(((k/kernel_cols-kernel_cols/2) * frame_cols) + (k%kernel_cols-kernel_cols/2));

  // ****** GRPPI code must be placed from here ***** //
  // loop throughtout every pixel
  for(auto it = frame.begin(), it2 = result.begin(); it != frame.end(); it++, it2++) 
  {
    // apply kernel to one pixel + its neightbourgs
    int weight = 0;
    int value = 0;
    for(int k = 0; k < kernel.size(); k++) {
      if ((it+offset[k] >= frame.begin()) && (it+offset[k] < frame.end())) {
        value += (*(it+offset[k]) * kernel[k]);
        weight += kernel[k];
      }
    }
    (*it2) = (unsigned char) (value / weight);
  } 
  // ****** to here ***** //
  return std::move(result);
}

grppi::dynamic_execution execution_mode(const std::string & opt) 
{
  using namespace grppi;
  if ("seq" == opt) return sequential_execution{};
  if ("thr" == opt) return parallel_execution_native{};
  if ("omp" == opt) return parallel_execution_omp{};
  if ("tbb" == opt) return parallel_execution_tbb{};
  return {};
}

void load_kernel(std::string kernel_file, std::vector<int>& kernel, int& kernel_cols)
{
  std::vector<int> ker;
  std::ifstream ifile( kernel_file );
  if( !ifile.is_open() ) {
    std::cerr << "Error: can't open file " << kernel_file << std::endl;
    std::exit(-1);
  }
  std::istream_iterator<int> input(ifile);
  std::copy(input, std::istream_iterator<int>(), std::back_inserter(ker));
  ifile.close();
  if ( ker.size() % 2 == 0 ) {
    std::cerr << "Error: kernel should have odd size" << std::endl;
    std::exit(-1);
  }
  kernel_cols= ker.size();
  for (auto i : ker)
    for (auto j : ker) 
      kernel.push_back(i*j);
}

void load_bmp(std::string input_file,
  int &width, int &height,
  std::vector<unsigned char> &header_info,
  std::vector<unsigned char> &red,
  std::vector<unsigned char> &green,
  std::vector<unsigned char> &blue)
{
  // open BMP input file
  std::ifstream input( input_file, std::ios::binary );
  if( !input.is_open() )  {
    std::cerr << "Can't open file" << std::endl;
    return;
  }

  // save BMP input file to a buffer
  std::vector<unsigned char> buffer( ( std::istreambuf_iterator<char>( input ) ),
   ( std::istreambuf_iterator<char>() ) );
  input.close();

  // load header file from buffer
  auto it = std::next( buffer.begin(), 14 );
  std::move( buffer.begin(), it, std::back_inserter( header_info ) );
  buffer.erase( buffer.begin(), it );

  // parse header file -> "BM <4:size> 0 0 0 0 <4:offset_pixels>"
  auto file_size = *reinterpret_cast<int*>( (unsigned char*)header_info.data() + 2 );
  auto pixel_offset = *reinterpret_cast<int*>( (unsigned char*)header_info.data() + 10 );

  // load image header from buffer
  it = std::next( buffer.begin(), pixel_offset-14 );
  std::move( buffer.begin(), it, std::back_inserter( header_info ) );
  buffer.erase( buffer.begin(), it );

  // get data from header
  auto img_header_size = *reinterpret_cast<int*>( (unsigned char*)header_info.data() + 4 );
  width = *reinterpret_cast<int*>( (unsigned char*)header_info.data() + 18 );
  height = *reinterpret_cast<int*>( (unsigned char*)header_info.data() + 22 );
  auto planes = *reinterpret_cast<short int*>( (unsigned char*)header_info.data() + 26 );
  auto bit_count = *reinterpret_cast<short int*>( (unsigned char*)header_info.data() + 28 );
  auto compresion = *reinterpret_cast<int*>( (unsigned char*)header_info.data() + 30 );
  auto image_size = *reinterpret_cast<int*>( (unsigned char*)header_info.data() + 34 );
  auto x_pxl_meter = *reinterpret_cast<int*>( (unsigned char*)header_info.data() + 38 );
  auto y_pxl_meter = *reinterpret_cast<int*>( (unsigned char*)header_info.data() + 42 );
  auto crl_used = *reinterpret_cast<int*>( (unsigned char*)header_info.data() + 46 );
  auto crl_important = *reinterpret_cast<int*>( (unsigned char*)header_info.data() + 50 );

  if ((planes != 1) || (bit_count != 24) || (compresion != 0)) {
    std::cerr << "Error: BMP incorrect format" << std::endl;
    std::exit(-1);
  }

  // get image from buffer
  int index = 0;
  for(auto row=0; row < height; row++){
    for(auto colum=0; colum < width; colum++){
      blue.push_back(buffer[index]);
      green.push_back(buffer[index+1]);
      red.push_back(buffer[index+2]);
      index += 3;
    }
    if ( 0 != (width*3) % 4 ) {
      index += 4 - ((width*3) % 4);
    }
  }
  it = std::next( buffer.begin(), index );
  buffer.erase( buffer.begin(), it);
}

void save_bmp(std::string output_file,
              int &width, int &height,
              std::vector<unsigned char> &header_info,
              std::vector<unsigned char> &red,
              std::vector<unsigned char> &green,
              std::vector<unsigned char> &blue)
{
  // open BMP output file
  std::ofstream arrayfile( output_file ); // File Creation
  std::ostream_iterator<unsigned char> output_iterator( arrayfile );
    
  // copy header to output file
  for(auto index=0; index<header_info.size(); index++) {
    arrayfile << header_info[index];
  }
  // copy image to output file
  int size = 3 * width * height;
  for(auto row=0; row < height; row++){
    for(auto colum=0; colum < width; colum++){
      auto index = (row * width) + colum;
      arrayfile << blue[index] << green[index] << red[index];
    }
    if ( 0 != (width*3) % 4 ) {
      for (int i=0; i < (4 - (width*3)%4); i++) {
        arrayfile << (unsigned char) 0;
      }
    }
  }
}

int main(int argc, char *argv[])
{
  // parameters checking
  if(argc != 5){
    std::cout << "Usage: " << argv[0]
              << " kernel input output mode" << std::endl;
    return -1;
  }

  std::string kernel_file(argv[1]), 
  input_file(argv[2]),
  output_file(argv[3]);
  auto exec = execution_mode(argv[4]);

  std::vector<int> kernel;
  std::vector<unsigned char> header_info, red, green, blue;
  int width=0, height=0, kernel_cols;
  std::chrono::time_point<std::chrono::system_clock> start, end;

  // load convolution kernel    
  load_kernel(kernel_file, kernel, kernel_cols);
  // load bmp image
  load_bmp(input_file, width, height, header_info, red, green, blue);

  // execute blur filter measuring execution time    
  start = std::chrono::system_clock::now();

  auto result_red   = blur(red,   width, kernel, kernel_cols, exec);
  auto result_green = blur(green, width, kernel, kernel_cols, exec);
  auto result_blue  = blur(blue,  width, kernel, kernel_cols, exec);

  end = std::chrono::system_clock::now();

  // save bmp image
  save_bmp(output_file, width, height, header_info,
   result_red, result_green, result_blue);

  // print preformance results
  int elapsed_seconds = std::chrono::duration_cast
    <std::chrono::milliseconds>(end-start).count();

  std::cout << "Execution time: " << elapsed_seconds << " milliseconds" << std::endl;

  return 0;
}