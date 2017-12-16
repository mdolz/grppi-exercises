add_executable(blur_seq blur_seq.cpp)
add_executable(blur_grppi blur_grppi.cpp)

target_link_libraries(blur_grppi ${GRPPI_LIBS})

file(COPY lena.bmp DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY kernel_avg3.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY kernel_avg5.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY kernel_avg7.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY kernel_gauss5.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
