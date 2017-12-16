add_executable(mergesort_seq mergesort_seq.cpp)
add_executable(mergesort_grppi mergesort_grppi.cpp)

target_link_libraries(mergesort_grppi ${GRPPI_LIBS})
