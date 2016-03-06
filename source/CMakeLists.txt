cmake_minimum_required(VERSION 2.4)
project( shared_allocator )

set(Source_Files_src 
  shared_allocator.cpp
)

source_group(Sources FILES ${Source_Files_src})

set(SOURCES 
  ${Source_Files_src}
)

add_library( ${PROJECT_NAME} SHARED ${SOURCES} )
