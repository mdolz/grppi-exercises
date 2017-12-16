/**
* @version      Mandelbrot Image Sequential - GrPPI v0.3
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

#include <complex>
#include <iostream>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sys/ioctl.h>
#include <string>
#include <cmath>

constexpr auto max_iteration = 1000;
typedef struct { unsigned char r, g, b; } color;
int mandelbrot_pixel(std::complex<double> start); 
color get_color(int iterations);

auto mandelbrot(int width, int height)
{
  double poi_x = -0.7, poi_y = 0.0;  // Point of interest
  double zoom = 0.003; // Mandelbrot zoom

  std::vector<color> image;
  for (int row= 0; row < height; row++) {
    for (int col= 0; col < width; col++) {
      std::complex<double> c{ col * zoom + (poi_x - ((width / 2.0) * zoom)),
                              row * zoom + (poi_y - ((height / 2.0) * zoom)) };
      image.push_back( get_color( mandelbrot_pixel(c) ) );
    }
  }
  return std::move(image);
}

int mandelbrot_pixel(std::complex<double> start) 
{
  int iterations = 0;
  std::complex<double> z;
  while (abs(z) < 2 && ++iterations < max_iteration) {
    z = pow(z, 2) + start;
  }
  return iterations;
}

color get_color(int iterations) 
{
    iterations = (iterations*127)/max_iteration;
    int r, g, b;

    if (iterations == -1) {
        r = 0;
        g = 0;
        b = 0;
    } else if (iterations == 0) {
        r = 255;
        g = 0;
        b = 0;
    } else if (iterations < 16) {
        r = 16 * (16 - iterations);
        g = 0;
        b = 16 * iterations - 1;
    } else if (iterations < 32) {
        r = 0;
        g = 16 * (iterations - 16);
        b = 16 * (32 - iterations) - 1;
    } else if (iterations < 64) {
        r = 8 * (iterations - 32);
        g = 8 * (64 - iterations) - 1;
        b = 0;
    } else { // range is 64 - 127
        r = 255 - (iterations - 64) * 4;
        g = 0;
        b = 0;
    }
    return color{(unsigned char)r, (unsigned char)g, (unsigned char)b};
}

void save_bmp(std::string filename, 
              int width, int height, 
              std::vector<color>& img)
{
  unsigned char file[14] = {
    'B','M', // magic
    0,0,0,0, // size in bytes
    0,0, // app data
    0,0, // app data
    40+14,0,0,0 // start of data offset
  };
  unsigned char info[40] = {
    40,0,0,0, // info hd size
    0,0,0,0, // width
    0,0,0,0, // heigth
    1,0, // number color planes
    24,0, // bits per pixel
    0,0,0,0, // compression is none
    0,0,0,0, // image bits size
    0x13,0x0B,0,0, // horz resoluition in pixel / m
    0x13,0x0B,0,0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
    0,0,0,0, // #colors in pallete
    0,0,0,0, // #important colors
  };

  int padSize  = (4-(width*3)%4)%4;
  int sizeData = width*height*3 + height*padSize;
  int sizeAll  = sizeData + sizeof(file) + sizeof(info);

  file[ 2] = (unsigned char)( sizeAll    );
  file[ 3] = (unsigned char)( sizeAll>> 8);
  file[ 4] = (unsigned char)( sizeAll>>16);
  file[ 5] = (unsigned char)( sizeAll>>24);

  info[ 4] = (unsigned char)( width   );
  info[ 5] = (unsigned char)( width>> 8);
  info[ 6] = (unsigned char)( width>>16);
  info[ 7] = (unsigned char)( width>>24);

  info[ 8] = (unsigned char)( height    );
  info[ 9] = (unsigned char)( height>> 8);
  info[10] = (unsigned char)( height>>16);
  info[11] = (unsigned char)( height>>24);

  info[20] = (unsigned char)( sizeData    );
  info[21] = (unsigned char)( sizeData>> 8);
  info[22] = (unsigned char)( sizeData>>16);
  info[23] = (unsigned char)( sizeData>>24);

  std::fstream stream;
  stream.open( filename, std::fstream::out );
  stream.write( (char*)file, sizeof(file) );
  stream.write( (char*)info, sizeof(info) );

  unsigned char pad[3] = {0,0,0};

  for ( int y=0; y<height; y++ ) {
    for ( int x=0; x<width; x++ ) {
      unsigned char pixel[3];
      pixel[0] = img[y*width+x].b;
      pixel[1] = img[y*width+x].g;
      pixel[2] = img[y*width+x].r;
      stream.write( (char*)pixel, 3 );
    }
    stream.write( (char*)pad, padSize );
  }
  stream.close();
}

int main(int argc, char *argv[])
{
  // parameters checking
  if(argc != 4){
    std::cout << "Usage: " << argv[0] 
              << " width height output" << std::endl;
    return -1;
  }
  int width{std::stoi(argv[1])};
  int height{std::stoi(argv[2])};    
  std::string output_file{argv[3]};

  std::chrono::time_point<std::chrono::system_clock> start, end;

  // execute mandelbrot measuring execution time    
  start = std::chrono::system_clock::now();
  auto image = mandelbrot(width, height);
  end = std::chrono::system_clock::now();

  // save bmp image
  save_bmp(output_file, width, height, image);

  // print preformance results
  int elapsed_seconds = 
    std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    
  std::cout << "Execution time: " << elapsed_seconds << " milliseconds" << std::endl;
    
  return 0;
}
