if (NOT EXISTS ${CMAKE_BINARY_DIR}/external/grppi)
  message("--grppi not found. Installing")

  ExternalProject_Add(grppi
    GIT_REPOSITORY "https://github.com/arcosuc3m/grppi.git"
    GIT_TAG "master"
    PREFIX external/grppi
    BUILD_IN_SOURCE
    UPDATE_COMMAND ""
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
      -DGRPPI_DOXY_ENABLE:BOOL=OFF
      -DGRPPI_APPLICATIONS_ENABLE:BOOL=OFF
      -DGRPPI_OMP_ENABLE:BOOL=ON
      -DGRPPI_TBB_ENABLE:BOOL=ON
      -DGRPPI_UNIT_TEST_ENABLE:BOOL=OFF
      -DCMAKE_BUILD_TYPE:STRING=Release
  )  
endif()

set(GRPPI_ROOT ${CMAKE_BINARY_DIR}/external/grppi)
set(env(GRPPI_ROOT) ${CMAKE_BINARY_DIR}/external/grppi)
