add_executable(sensor_seq sensor_seq.cpp)
add_executable(sensor_grppi sensor_grppi.cpp)

target_link_libraries(sensor_grppi ${GRPPI_LIBS})
