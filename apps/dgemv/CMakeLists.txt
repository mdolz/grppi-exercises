add_executable(dgemv_seq dgemv_seq.cpp)
add_executable(dgemv_grppi dgemv_grppi.cpp)

target_link_libraries(dgemv_grppi ${GRPPI_LIBS})
