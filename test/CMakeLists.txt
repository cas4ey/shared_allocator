cmake_minimum_required(VERSION 2.4)
project( shared_allocator_test )

set(Source_Files_src 
  main.cpp
)

set(Include_Files_src 
  ${ROOT}/include/shared_allocator/shared_allocator.hpp
  ${ROOT}/include/shared_allocator/cached_allocator.hpp
  ${ROOT}/include/shared_allocator/size_cached_allocator.hpp
)

source_group(Sources FILES ${Source_Files_src})
source_group(Includes FILES ${Include_Files_src})

set(SOURCES 
  ${Source_Files_src}
  ${Include_Files_src}
)

add_executable( ${PROJECT_NAME} ${SOURCES} )

target_link_libraries( ${PROJECT_NAME} shared_allocator)
