add_executable(mandelbrot_seq mandelbrot_seq.cpp)
add_executable(mandelbrot_grppi mandelbrot_grppi.cpp)

target_link_libraries(mandelbrot_grppi ${GRPPI_LIBS})
