add_executable(mandelbrot_video_seq mandelbrot_video_seq.cpp)
add_executable(mandelbrot_video_grppi mandelbrot_video_grppi.cpp)

target_link_libraries(mandelbrot_video_grppi ${GRPPI_LIBS})
