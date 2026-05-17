cmake_minimum_required(VERSION 3.21)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE AND NOT DEFINED CMAKE_OSX_ARCHITECTURES)
  set(CMAKE_OSX_ARCHITECTURES "${CMAKE_HOST_SYSTEM_PROCESSOR}" CACHE STRING "" FORCE)
endif()

project(csound VERSION 1.0.1 LANGUAGES CXX)

set(CSOUND_NWJS_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}"
    CACHE PATH "csound-nwjs source directory."
)

message(STATUS "CMAKE_HOST_SYSTEM_NAME: ${CMAKE_HOST_SYSTEM_NAME}")

# -----------------------------------------------------------------------------
# Refresh FindCsoundHomeFirst.cmake from csound-ac
# -----------------------------------------------------------------------------
message(STATUS "Refreshing FindCsoundHomeFirst.cmake from csound-ac (csound_threaded.hpp is downloaded from GitHub head at build time)...")

file(MAKE_DIRECTORY "${CSOUND_NWJS_SOURCE_DIR}/cmake")

set(csound_ac_refresh_dir "${CMAKE_BINARY_DIR}/csound-ac-refresh")
file(MAKE_DIRECTORY "${csound_ac_refresh_dir}")

file(DOWNLOAD
    "https://raw.githubusercontent.com/gogins-dev/csound-ac/refs/heads/master/cmake/FindCsoundHomeFirst.cmake"
    "${csound_ac_refresh_dir}/FindCsoundHomeFirst.cmake"
    STATUS find_csound_download_status
    SHOW_PROGRESS
)

list(GET find_csound_download_status 0 find_csound_download_code)

if(find_csound_download_code EQUAL 0)
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
      "${csound_ac_refresh_dir}/FindCsoundHomeFirst.cmake"
      "${CSOUND_NWJS_SOURCE_DIR}/cmake/FindCsoundHomeFirst.cmake"
  )
else()
  list(GET find_csound_download_status 1 find_csound_download_message)
  message(WARNING
    "Failed to refresh FindCsoundHomeFirst.cmake from csound-ac head: "
    "${find_csound_download_message}. Using local copy if present."
  )
endif()

# -----------------------------------------------------------------------------
# Csound (home-first finder; optional CSOUND_ROOT_HINT / env CSOUND_ROOT)
# -----------------------------------------------------------------------------
if(DEFINED ENV{CSOUND_ROOT} AND NOT DEFINED CSOUND_ROOT_HINT)
  set(CSOUND_ROOT_HINT "$ENV{CSOUND_ROOT}" CACHE PATH "Prefix where Csound 7 is installed (bin, lib, include)." FORCE)
endif()

set(CSOUND_ROOT_HINT "" CACHE PATH "Prefix where Csound 7 is installed (overrides env CSOUND_ROOT).")

list(APPEND CMAKE_MODULE_PATH "${CSOUND_NWJS_SOURCE_DIR}/cmake")
set(FIND_CSOUND_HOME_FIRST_REQUIRED ON)
include("${CSOUND_NWJS_SOURCE_DIR}/cmake/FindCsoundHomeFirst.cmake")

if(NOT CSOUND_FOUND)
  message(FATAL_ERROR "Csound 7 was not found. Set CSOUND_ROOT or -DCSOUND_ROOT_HINT=... to your install prefix.")
endif()

if(CSOUND_INCLUDE_DIR AND EXISTS "${CSOUND_INCLUDE_DIR}/csound/csound.hpp")
  list(APPEND CSOUND_INCLUDE_DIRS "${CSOUND_INCLUDE_DIR}/csound")
endif()

# -----------------------------------------------------------------------------
# CsoundAC (libCsoundAC + headers): built in-tree next to this repo or explicit paths
# -----------------------------------------------------------------------------
set(CSOUND_AC_ROOT "" CACHE PATH "Path to csound-ac repository root (contains CsoundAC/).")
set(CSOUNDAC_LIBRARY "" CACHE FILEPATH "Path to libCsoundAC (.dylib / .so / .dll / .lib).")
set(CSOUND_AC_INCLUDE_DIR "" CACHE PATH "Directory containing CsoundProducer.hpp (usually .../CsoundAC).")

# Prefer cache (-D / cmake-js --CD); else environment; else sibling clone ../csound-ac.
if(NOT CSOUND_AC_ROOT)
  if(DEFINED ENV{CSOUND_AC_ROOT} AND NOT "$ENV{CSOUND_AC_ROOT}" STREQUAL "")
    set(CSOUND_AC_ROOT "$ENV{CSOUND_AC_ROOT}" CACHE PATH "Path to csound-ac repository root (from env CSOUND_AC_ROOT)." FORCE)
  endif()
endif()

if(NOT CSOUND_AC_ROOT)
  get_filename_component(_nwjs_csound_ac_sibling "${CMAKE_CURRENT_SOURCE_DIR}/../csound-ac" ABSOLUTE)
  if(EXISTS "${_nwjs_csound_ac_sibling}/CsoundAC/CsoundProducer.hpp")
    set(CSOUND_AC_ROOT "${_nwjs_csound_ac_sibling}" CACHE PATH "Path to csound-ac repository root (auto: sibling ../csound-ac)." FORCE)
    message(STATUS "CSOUND_AC_ROOT (default): ${CSOUND_AC_ROOT}")
  endif()
endif()

if(NOT CSOUND_AC_INCLUDE_DIR)
  if(DEFINED ENV{CSOUND_AC_INCLUDE_DIR} AND NOT "$ENV{CSOUND_AC_INCLUDE_DIR}" STREQUAL "")
    set(CSOUND_AC_INCLUDE_DIR "$ENV{CSOUND_AC_INCLUDE_DIR}" CACHE PATH "CsoundAC headers directory (from env)." FORCE)
  endif()
endif()

if(CSOUND_AC_ROOT AND NOT CSOUND_AC_INCLUDE_DIR)
  set(CSOUND_AC_INCLUDE_DIR "${CSOUND_AC_ROOT}/CsoundAC" CACHE PATH "CsoundAC headers directory." FORCE)
endif()

if(NOT CSOUND_AC_INCLUDE_DIR)
  message(
    FATAL_ERROR
    "CsoundAC headers not found. Set one of:\n"
    "  - CMake: -DCSOUND_AC_ROOT=/path/to/csound-ac  or  -DCSOUND_AC_INCLUDE_DIR=/path/to/CsoundAC\n"
    "  - Environment: CSOUND_AC_ROOT or CSOUND_AC_INCLUDE_DIR\n"
    "  - Or clone csound-ac next to this repo as ../csound-ac\n"
    "  - Or run ./rebuild.bash which sets cmake-js --CDCSOUND_AC_ROOT=..."
  )
endif()

if(NOT CSOUNDAC_LIBRARY)
  find_library(
    CSOUNDAC_LIBRARY
    NAMES CsoundAC libCsoundAC libCsoundAC.8.0 libCsoundAC.7.1 libCsoundAC.7.0
    PATHS
      "${CSOUND_AC_ROOT}/build-macos"
      "${CSOUND_AC_ROOT}/build-linux"
      "${CSOUND_AC_ROOT}/build-windows"
      "${CSOUND_AC_ROOT}/build"
      "${CSOUND_AC_ROOT}/build/CsoundAC"
      "${CSOUND_AC_ROOT}/build-macos/CsoundAC"
      "${CSOUND_AC_ROOT}/build-linux/CsoundAC"
      "${CSOUND_AC_ROOT}/build-windows/CsoundAC"
      "${CSOUND_AC_ROOT}/dist/csound-ac/lib"
      "${CMAKE_PREFIX_PATH}"
    NO_DEFAULT_PATH
  )
endif()

# Versioned dylibs often live directly under build-macos/ (not .../CsoundAC/).
if(NOT CSOUNDAC_LIBRARY AND CSOUND_AC_ROOT)
  file(
    GLOB _nwjs_libcsoundac
    LIST_DIRECTORIES false
    "${CSOUND_AC_ROOT}/build-macos/libCsoundAC*.dylib"
    "${CSOUND_AC_ROOT}/build-linux/libCsoundAC.so*"
    "${CSOUND_AC_ROOT}/build/CsoundAC/libCsoundAC*.dylib"
    "${CSOUND_AC_ROOT}/build/CsoundAC/libCsoundAC.so*"
    "${CSOUND_AC_ROOT}/build-macos/CsoundAC/libCsoundAC*.dylib"
    "${CSOUND_AC_ROOT}/build-linux/CsoundAC/libCsoundAC.so*"
    "${CSOUND_AC_ROOT}/dist/csound-ac/lib/libCsoundAC*.dylib"
    "${CSOUND_AC_ROOT}/dist/csound-ac/lib/libCsoundAC.so*"
  )
  list(LENGTH _nwjs_libcsoundac _nwjs_libcsoundac_len)
  if(_nwjs_libcsoundac_len GREATER 0)
    list(GET _nwjs_libcsoundac 0 CSOUNDAC_LIBRARY)
    message(STATUS "Found libCsoundAC via GLOB under CSOUND_AC_ROOT: ${CSOUNDAC_LIBRARY}")
  endif()
endif()

if(NOT CSOUNDAC_LIBRARY)
  find_library(CSOUNDAC_LIBRARY NAMES CsoundAC libCsoundAC libCsoundAC.8.0 libCsoundAC.7.1 libCsoundAC.7.0)
endif()

if(NOT CSOUNDAC_LIBRARY)
  message(
    FATAL_ERROR
    "libCsoundAC not found under CSOUND_AC_ROOT=${CSOUND_AC_ROOT}. "
    "Build csound-ac (e.g. Ninja target libCsoundAC) or pass -DCSOUNDAC_LIBRARY=/path/to/libCsoundAC.*"
  )
endif()

message(STATUS "Using CsoundAC library: ${CSOUNDAC_LIBRARY}")
message(STATUS "Using CsoundAC includes: ${CSOUND_AC_INCLUDE_DIR}")

# -----------------------------------------------------------------------------
# Node / N-API: cmake-js sets CMAKE_JS_INC to Node's include/node only; napi.h is in node-addon-api.
# -----------------------------------------------------------------------------
set(_napi_include "")
if(DEFINED ENV{NODE_ADDON_API_INCLUDE} AND NOT "$ENV{NODE_ADDON_API_INCLUDE}" STREQUAL "")
  set(_napi_include "$ENV{NODE_ADDON_API_INCLUDE}")
elseif(NODE_ADDON_API_DIR)
  set(_napi_include "${NODE_ADDON_API_DIR}")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/native/node_modules/node-addon-api/napi.h")
  set(_napi_include "${CMAKE_CURRENT_SOURCE_DIR}/native/node_modules/node-addon-api")
endif()

# Strip wrapping double quotes if NODE_ADDON_API_INCLUDE used node-addon-api's deprecated .include value.
string(STRIP "${_napi_include}" _napi_include)
string(REGEX REPLACE "^\"(.*)\"$" "\\1" _napi_include "${_napi_include}")

if(NOT DEFINED CMAKE_JS_INC OR CMAKE_JS_INC STREQUAL "")
  if(_napi_include)
    set(CMAKE_JS_INC "${_napi_include}")
  else()
    message(
      FATAL_ERROR
        "Node headers not found (CMAKE_JS_INC). Use cmake-js, or set NODE_ADDON_API_INCLUDE / NODE_ADDON_API_DIR, "
        "or add native/package.json and run (cd native && npm install)."
    )
  endif()
endif()

if(NOT _napi_include)
  message(
    FATAL_ERROR
      "node-addon-api not found (napi.h). Export NODE_ADDON_API_INCLUDE=/path/to/node-addon-api, "
      "pass -DNODE_ADDON_API_DIR=..., or run (cd native && npm install) so native/node_modules/node-addon-api exists."
  )
endif()

set(_cmake_js_src "")
if(DEFINED CMAKE_JS_SRC)
  set(_cmake_js_src ${CMAKE_JS_SRC})
endif()

file(GLOB SOURCE_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${_cmake_js_src})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node")

# csound-ac CsoundAC/csound_threaded.hpp: always download from GitHub head
# into the source-tree copy before compiling. This source directory is listed
# before CSOUND_AC_INCLUDE_DIR / CSOUND includes so the refreshed file wins over
# any installed csound_threaded.hpp.
set(CSOUND_NWJS_CSOUND_THREADED_INCLUDE_DIR "${CSOUND_NWJS_SOURCE_DIR}")
set(CSOUND_AC_CSOUND_THREADED_URL
    "https://raw.githubusercontent.com/gogins-dev/csound-ac/refs/heads/master/CsoundAC/csound_threaded.hpp"
)
set(CSOUND_AC_CSOUND_THREADED_DST "${CSOUND_NWJS_SOURCE_DIR}/csound_threaded.hpp")
# Single -D arguments avoid Ninja/VERBATIM edge cases with empty URL for cmake -P.
string(CONCAT _csound_threaded_url_arg "-DURL=" "${CSOUND_AC_CSOUND_THREADED_URL}")
string(CONCAT _csound_threaded_dst_arg "-DDST=" "${CSOUND_AC_CSOUND_THREADED_DST}")
add_custom_target(
  csound_ac_fetch_csound_threaded
  COMMAND "${CMAKE_COMMAND}" "${_csound_threaded_url_arg}" "${_csound_threaded_dst_arg}"
          -P "${CSOUND_NWJS_SOURCE_DIR}/cmake/FetchCsoundACCsoundThreaded.cmake"
  COMMENT "Download csound-ac CsoundAC/csound_threaded.hpp from GitHub head"
  VERBATIM
)
add_dependencies(${PROJECT_NAME} csound_ac_fetch_csound_threaded)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
    "${CMAKE_JS_INC}"
    "${_napi_include}"
    "${CSOUND_NWJS_CSOUND_THREADED_INCLUDE_DIR}"
    ${CSOUND_INCLUDE_DIRS}
    "${CSOUND_AC_INCLUDE_DIR}"
)

if(NOT CSOUND_VERSION_MAJOR)
  set(CSOUND_VERSION_MAJOR 7)
endif()

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE
    NAPI_VERSION=4
    CSOUND_VERSION_MAJOR=${CSOUND_VERSION_MAJOR}
)

if(APPLE)
  target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
      ${CMAKE_JS_LIB}
      ${CSOUND_LIBRARIES}
      ${CSOUNDAC_LIBRARY}
      pthread
  )
  target_link_options(
    ${PROJECT_NAME}
    PRIVATE
      "LINKER:-rpath,@loader_path"
      "LINKER:-rpath,/Library/Frameworks"
      "LINKER:-rpath,/usr/local/lib"
  )
  if(CSOUND_FRAMEWORK_DIR)
    get_filename_component(_csound_fw_parent "${CSOUND_FRAMEWORK_DIR}" DIRECTORY)
    target_link_options(${PROJECT_NAME} PRIVATE "LINKER:-F${_csound_fw_parent}")
  endif()
  get_filename_component(_csoundac_dir "${CSOUNDAC_LIBRARY}" DIRECTORY)
  if(_csoundac_dir)
    target_link_options(${PROJECT_NAME} PRIVATE "LINKER:-rpath,${_csoundac_dir}")
  endif()
elseif(UNIX AND NOT APPLE)
  target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
      ${CMAKE_JS_LIB}
      ${CSOUND_LIBRARIES}
      ${CSOUNDAC_LIBRARY}
      pthread
  )
  get_filename_component(_csoundac_dir "${CSOUNDAC_LIBRARY}" DIRECTORY)
  if(_csoundac_dir)
    target_link_options(${PROJECT_NAME} PRIVATE "LINKER:-rpath,${_csoundac_dir}")
  endif()
elseif(WIN32)
  if(MSVC AND DEFINED CMAKE_JS_NODELIB_DEF AND DEFINED CMAKE_JS_NODELIB_TARGET)
    execute_process(
      COMMAND ${CMAKE_AR} /def:${CMAKE_JS_NODELIB_DEF} /out:${CMAKE_JS_NODELIB_TARGET} ${CMAKE_STATIC_LINKER_FLAGS}
    )
  endif()
  target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
      ${CMAKE_JS_LIB}
      ${CSOUND_LIBRARIES}
      ${CSOUNDAC_LIBRARY}
  )
endif()
