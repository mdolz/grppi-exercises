/**
* @version      Mandelbrot Video Parallel - GrPPI v0.3
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
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <sys/ioctl.h>
#include <string>
#include <signal.h>
#include <iomanip>
#include "grppi.h"
#include "dyn/dynamic_execution.h"

using namespace std::chrono;

char ascii_map[70]= {'$','@','B','%','8','&','W','M','#',
                     ',','*','o','a','h','k','b','d','p',
                     'q','w','m','Z','O','0','Q','L','C',
                     'J','U','Y','X','z','c','v','u','n',
                     'x','r','j','f','t','/','\\','|','(',
                     ')','1','{','}','[',']','?','-','_',
                     '+','~','<','>','i','!','l','I',';',
                     ':','"','^','`','\'','.',' '};
std::string color[3] = {"\033[1;33m", "\033[1;31m", "\033[1;35m"};

constexpr auto max_iteration = 1000, max_frames= 3000;

double mandelbrot_pixel(std::complex<double> start); 
std::string get_color(double iterations);
std::string get_stats(
  time_point<system_clock> start, 
  time_point<system_clock> end,
  time_point<system_clock> init,
  std::vector<time_point<system_clock>>& frame_times, 
  int& current, int& frames);

bool finalize = false;

void signal_callback_handler(int signum) { finalize = true; }

void mandelbrot(int width, int height,
  const grppi::dynamic_execution& exec)
{
  double poi_x = -0.0452407411, 
         poi_y = 0.9868162204352258;  // Point of interest
  double zoom = 1; // Mandelbrot zoom
  
  time_point<system_clock> next, init;
  std::vector<time_point<system_clock>> frame_times(11);
  int frames= 0, current= 0, generated_frames = 0;

  next = system_clock::now();
  init = next;
  std::cout << "\033[2J";

  // ****** GRPPI code must be placed from here ***** //
  while (!finalize && generated_frames++ <= max_frames) {
    // make no more than 30 frames per second
    std::this_thread::sleep_for(milliseconds(1000/30));
    zoom-= zoom * 0.01;

    std::stringstream image;
    for(auto row = 0; row < height; ++row){
      for(auto col = 0; col < width; ++col){
        std::complex<double> 
          c{ col * zoom + (poi_x - ((width  / 2.0) * zoom)),
             row * zoom + (poi_y - ((height / 2.0) * zoom)) };
        image << get_color( mandelbrot_pixel(c) );
      }
      image << "\n";
    }
    std::string im = image.str();

    std::cout << get_stats(next, system_clock::now(), 
                 init, frame_times, current, frames)
              << im << std::flush;
    next = system_clock::now();
  }
  // ****** to here ***** //
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

double mandelbrot_pixel(std::complex<double> start) 
{
  int iterations = 0;
  std::complex<double> z;
  while (abs(z) < 2 && ++iterations < max_iteration) {
    z = pow(z, 2) + start;
  }
  return iterations;
}

std::string get_color(double iterations) 
{
    iterations = 1 - iterations / (double) max_iteration;
    std::string color_{ color[(int)(iterations*(70*3))/70] },
                ascii_{ ascii_map[(int)(iterations*(70*3))%70] },
                end_{ "\033[0m" };
    return color_ + ascii_ + end_;
}

std::string get_stats(
  time_point<system_clock> start, 
  time_point<system_clock> end,
  time_point<system_clock> init,
  std::vector<time_point<system_clock>>& frame_times, 
  int& current, int& frames)
{
  frame_times[current] = end;
  double inst_fps = 0.0;

  if (frames > 11)
  {
    auto time= duration_cast<milliseconds>
                (frame_times[current] - 
                 frame_times[abs((current+1)%11)]).count()/1000.0;
    inst_fps= 10/time;
  }        

  frames++;
  current= (current+1)%11;

  double elapsed_seconds= (duration_cast<milliseconds>(end-start).count()/1000.0),
          execution_time= (duration_cast<milliseconds>(end-init).count()/1000.0);

  std::stringstream s;
  s << "\033[2J\033[1;1H" 
    << "[ FPS: " << std::left 
                 << std::setw(6) 
                 << std::setprecision(5) 
                 << inst_fps 
    << " ] [ Averaged FPS: " << std::left 
                 << std::setw(6) 
                 << std::setprecision(5) 
                 << frames/execution_time
    << " ] [ Frame nr.: " << std::right 
                 << std::setw(5)
                 << frames
                 << std::setprecision(4) 
    << " ] [ Elapsed time: " << std::left 
                 << std::setw(7)
                 << std::setprecision(6) 
                 << execution_time 
    << "]\n";
  return s.str();
}

int main (int argc, char *argv[])
{
  if(argc != 2){
    std::cout << "Usage: " << argv[0] 
              << " mode" << std::endl;
    return -1;
  }
  signal(SIGINT, signal_callback_handler);

  auto exec = execution_mode(argv[1]);
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  int width = w.ws_col - 3, height = w.ws_row - 3;

  mandelbrot(width, height, exec);

  return 0;
}
