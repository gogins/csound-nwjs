cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW) 
message(STATUS "CMAKE_HOST_SYSTEM_NAME:  ${CMAKE_HOST_SYSTEM_NAME}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "" FORCE)

project (csound)

#
# Find Csound version.
#
include("$ENV{HOME}/cmake/FindCsoundHomeFirst.cmake")

add_definitions(-DNAPI_VERSION=4)

include_directories(${CMAKE_JS_INC})

file(DOWNLOAD
    "https://raw.githubusercontent.com/gogins/csound-ac/master/CsoundAC/csound_threaded.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/csound_threaded.hpp"
)

file(GLOB SOURCE_FILES "*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

if (APPLE)
message(STATUS "Building for macOS.")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB}  
  /Library/Frameworks/CsoundLib64.framework/Versions/7.0/CsoundLib64 
  pthread
  )
target_include_directories(${PROJECT_NAME} PRIVATE 
  /opt/homebrew/lib/node_modules/node-addon-api/ 
  /Library/Frameworks/CsoundLib64.framework/Versions/7.0/Headers 
  /opt/homebrew/Cellar/ecl/21.2.1_1/include/ 
  /opt/homebrew/Cellar/gmp/6.2.1_1/include/ 
  /opt/homebrew/Cellar/bdw-gc/8.0.6/include/' 
  /Users/michaelgogins/csound-ac/CsoundAC/ 
  )
target_compile_definitions(${PROJECT_NAME} PRIVATE 
  NAPI_VERSION=4
  CSOUND_VERSION_MAJOR=7
)
endif()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
message(STATUS "Building for Linux.")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB}  
  csound64
  CsoundAC  
  pthread
  )
target_include_directories(${PROJECT_NAME} PRIVATE 
  ${CMAKE_SOURCE_DIR}/node_modules/node-addon-api 
  /usr/local/include/csound
  /usr/include/csound
  )
endif()

if(MSVC AND CMAKE_JS_NODELIB_DEF AND CMAKE_JS_NODELIB_TARGET)
  # Generate node.lib
  execute_process(COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS})
endif()