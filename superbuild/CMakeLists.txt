## Copyright 2019-2020 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

## Global settings ##

cmake_minimum_required(VERSION 3.1)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}/dependencies
)

if(NOT CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX
      "${CMAKE_BINARY_DIR}/install"
      CACHE STRING "Final install location." FORCE)
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

## ======================================================================== ##
## Project setup.
## ======================================================================== ##

project(OpenVKL_superbuild)

include(ExternalProject)
include(GNUInstallDirs)
include(ProcessorCount)

## Setup top-level variables ##

set(installDir ${CMAKE_INSTALL_PREFIX})

get_filename_component(INSTALL_DIR_ABSOLUTE
  ${installDir} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})

ProcessorCount(PROCESSOR_COUNT)

if(NOT PROCESSOR_COUNT EQUAL 0)
  set(BUILD_JOBS ${PROCESSOR_COUNT} CACHE STRING "Number of build jobs '-j <n>'")
else()
  set(BUILD_JOBS 4 CACHE STRING "Number of build jobs '-j <n>'")
endif()

if (${CMAKE_VERSION} VERSION_GREATER 3.11.4)
  set(PARALLEL_JOBS_OPTS -j ${BUILD_JOBS})
endif()

set(DEFAULT_BUILD_COMMAND ${CMAKE_COMMAND} --build . --config Release ${PARALLEL_JOBS_OPTS})

# We must replace separators so that ExternalProject_Add can work with the
# paths we generate.
macro(add_to_prefix_path PATH)
  list(APPEND CMAKE_PREFIX_PATH ${PATH})
  string(REPLACE ";" "|" CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}")
endmacro()

## ======================================================================== ##
## Superbuild options.
## ======================================================================== ##

option(INSTALL_IN_SEPARATE_DIRECTORIES
  "Install libraries into their own directories under CMAKE_INSTALL_PREFIX"
  OFF
)

option(BUILD_DEPENDENCIES_ONLY
  "Do not build Open VKL itself, only its dependencies" 
  OFF
)

option(BUILD_OPENVKL_TESTING
  "Enable testing targets when building Open VKL"
  ON
)

option(BUILD_OPENVKL_BENCHMARKS
  "Enable benchmark targets when building Open VKL"
  OFF
)

## ======================================================================== ##
## Build dependencies.
##
## Use the BUILD_* options to enable or disable building in superbuild. If 
## These options are OFF, then required dependencies must be available in the 
## environment.
##
## For most of the dependencies, you may override the *_URL and *_HASH
## variables to select different versions.
## ======================================================================== ##

option(BUILD_ISPC "Build the Intel SPMD Program Compiler or search in environment?" ON)
if (BUILD_ISPC)
  set(ISPC_VERSION "1.13.0")
  set(ISPC_BASE_URL "https://github.com/ispc/ispc/releases/download/v${ISPC_VERSION}")
  if (APPLE)
    set(_ISPC_URL "${ISPC_BASE_URL}/ispc-v${ISPC_VERSION}-macOS.tar.gz")
    set(_ISPC_HASH "0dc7eaf3335b299e262f052fb96f0ad5a4e04a41492f399b690ca788e0fd304b")
  elseif(WIN32)
    set(_ISPC_URL "${ISPC_BASE_URL}/ispc-v${ISPC_VERSION}-windows.zip")
    set(_ISPC_HASH "b0a3a96baf426845fd1a38ca5c1dcca094354bea99a3bc3f8e3a741bf07e6608")
  else()
    set(_ISPC_URL "${ISPC_BASE_URL}/ispc-v${ISPC_VERSION}-linux.tar.gz")
    set(_ISPC_HASH "8ab1189bd5db596b3eee9d9465d3528b6626a7250675d67102761bb0d284cd21")
  endif()
  set(ISPC_URL "${_ISPC_URL}" CACHE STRING "URL of the ISPC archive.")
  set(ISPC_HASH "${_ISPC_HASH}" CACHE STRING "SHA256 hash of the ISPC archive.")
  include(dep_ispc)
endif()


option(BUILD_TBB "Build Intel Threading Building Blocks or search in environment?" ON)
if (BUILD_TBB)
  set(TBB_VERSION "2020.2")
  set(TBB_BASE_URL "https://github.com/oneapi-src/oneTBB/releases/download/v${TBB_VERSION}/tbb-${TBB_VERSION}")
  if (APPLE)
    set(_TBB_URL "${TBB_BASE_URL}-mac.tgz")
    set(_TBB_HASH "4ca94512289e857a1d23d122947f2902c2edf6e521a5f29d161bf8458ea317b5")
    set(TBB_LIB_SUBDIR "")
  elseif(WIN32)
    set(_TBB_URL "${TBB_BASE_URL}-win.zip")
    set(_TBB_HASH "087b63903e3b3ea26cbf0c128c768ce8aefc23d18a3089ce929c0c8a6dfd0f64")
    set(TBB_LIB_SUBDIR "intel64/vc14")
  else()
    set(_TBB_URL "${TBB_BASE_URL}-lin.tgz")
    set(_TBB_HASH "b2545fbf162a1c16613add3463687154b967757bd0a00b70c885f7aef2a921d5")
    set(TBB_LIB_SUBDIR "intel64/gcc4.8")
  endif()
  set(TBB_URL "${_TBB_URL}" CACHE STRING "URL of the TBB archive.")
  set(TBB_HASH "${_TBB_HASH}" CACHE STRING "SHA256 hash of the TBB archive.")
  include(dep_tbb)
endif()


option(BUILD_RKCOMMON "Build rkcommon or search in environment?" ON)
if (BUILD_RKCOMMON)
  set(RKCOMMON_VERSION "v1.4.1" CACHE STRING "rkcommon version to download")
  set(RKCOMMON_URL "https://github.com/ospray/rkcommon/archive/${RKCOMMON_VERSION}.zip"
    CACHE STRING "URL of the rkcommon archive.")
  set(RKCOMMON_HASH "68a122e6da9f10441d6f1b85f5787b57be01008495a8ac025bd29fbd15359c57"
    CACHE STRING "SHA256 hash of the rkcommon archive.")
  include(dep_rkcommon)
endif()


# Embree can either be build from source or downloaded as binaries.
option(BUILD_EMBREE "Build Intel Embree or search in environment?" ON)
option(BUILD_EMBREE_FROM_SOURCE "Build Embree from source or use pre-built version? (Only used when BUILD_EMBREE=ON)" ON)
if (BUILD_EMBREE)
  set(EMBREE_VERSION "v3.11.0" CACHE STRING "Embree version to download")
  if (BUILD_EMBREE_FROM_SOURCE)
    set(_EMBREE_URL "https://github.com/embree/embree/archive/${EMBREE_VERSION}.zip")
    set(_EMBREE_HASH "2247a7b0560497d7b7e5e67e237a5cb8ed853b5e6484cd8995a87e746bc308bc")
  else()
    # Embree binary package URLs do not use the "v" prefix
    string(REPLACE "v" "" EMBREE_VERSION_NUMBER ${EMBREE_VERSION})
    set(EMBREE_BASE_URL "https://github.com/embree/embree/releases/download/${EMBREE_VERSION}")
    if (APPLE)
      set(_EMBREE_URL "${EMBREE_BASE_URL}/embree-${EMBREE_VERSION_NUMBER}.x86_64.macosx.zip")
      set(_EMBREE_HASH "81548ebc7689c6451be5409a9395bdf48a6259c0debdc43fe134bc0e5f306448")
    elseif (WIN32)
      set(_EMBREE_URL "${EMBREE_BASE_URL}/embree-${EMBREE_VERSION_NUMBER}.x64.vc14.windows.zip")
      set(_EMBREE_HASH "97c4cc85dd60127a81751a68aaaa5dba9e410faf558a6e08f2226484ffc67b7d")
    else()
      set(_EMBREE_URL "${EMBREE_BASE_URL}/embree-${EMBREE_VERSION_NUMBER}.x86_64.linux.tar.gz")
      set(_EMBREE_HASH "f61ecc7401b8c9444008d53201ec681516f048ee31d43d125dad917fc76c5b71")
    endif()
  endif()
  set(EMBREE_URL "${_EMBREE_URL}"  CACHE STRING "URL of the Embree source archive.")
  set(EMBREE_HASH "${_EMBREE_HASH}" CACHE STRING "SHA256 hash for the Embree source archive.")
  include(dep_embree)
endif()


option(BUILD_GLFW "Build glfw or search in environment?" ON)
if (BUILD_GLFW)
  set(GLFW_VERSION "3.2.1")
  set(GLFW_URL "https://github.com/glfw/glfw/archive/${GLFW_VERSION}.zip"
    CACHE STRING "URL of the GLFW source archive.")
  set(GLFW_HASH "0c623f65a129c424d0fa45591694fde3719ad4a0955d4835182fda71b255446f" 
    CACHE STRING "SHA256 hash of the GLFW source archive.")
  include(dep_glfw)
endif()


# We always build or download OpenVDB and all its dependencies if OpenVDB is
# enabled, and to not attempt to find them in the environment.
set(BUILD_OPENVDB_DEFAULT ON)
mark_as_advanced(BUILD_OPENVDB_DEFAULT)
if (WIN32)
  set(BUILD_OPENVDB_DEFAULT OFF)
endif()

option(BUILD_OPENVDB "Enable OpenVDB to support loading .vdb files?" ${BUILD_OPENVDB_DEFAULT})
option(BUILD_BLOSC "Support loading compressed .vdb files using c-BLOSC? (Only used when BUILD_OPENVDB=ON)" ON)
if (BUILD_OPENVDB)
  if (WIN32)
    message(FATAL_ERROR "Open VKL superbuild currently does not support building on Windows.")
    set(BUILD_OPENVDB OFF)
  elseif (CMAKE_VERSION VERSION_LESS 3.7)
    message(WARNING "Disabling OpenVDB support because the cmake version is too low (3.7 is required)")
    set(BUILD_OPENVDB OFF)
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.3)
    message(WARNING "Disabling OpenVDB support because the gcc compiler version is too low (6.3 is required)")
    set(BUILD_OPENVDB OFF)
  elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.1)
    message(WARNING "Disabling OpenVDB support because the icc compiler version is too low (19.1 is required)")
    set(BUILD_OPENVDB OFF)
  else()
    set(ILMBASE_VERSION "2.4.1")
    set(ILMBASE_URL "https://github.com/AcademySoftwareFoundation/openexr/archive/v${ILMBASE_VERSION}.zip"
      CACHE STRING "URL of the IlmBase archive.")
    set(ILMBASE_HASH "fd828641897f4ceb9277b61e3add25c3306adcb4670355ac7081d78cf7ee8f5f"
      CACHE STRING "SHA256 hash of the IlmBase archive.")
    include(dep_ilmbase)

    set(ZLIB_VERSION "1.2.11")
    set(ZLIB_URL "https://www.zlib.net/zlib-${ZLIB_VERSION}.tar.gz"
      CACHE STRING "URL of the zlib archive.")
    set(ZLIB_HASH "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1" 
      CACHE STRING "SHA256 hash of the zlib archive.")
    include(dep_zlib)

    set(BOOST_VERSION "1.72.0")
    string(REPLACE "." "_" BOOST_FILE_BASE "${BOOST_VERSION}")
    set(BOOST_BASE_URL "https://dl.bintray.com/boostorg/release/${BOOST_VERSION}/source/boost")
    set(_BOOST_URL "${BOOST_BASE_URL}_${BOOST_FILE_BASE}.tar.gz")
    set(_BOOST_HASH "c66e88d5786f2ca4dbebb14e06b566fb642a1a6947ad8cc9091f9f445134143f")
    set(BOOST_URL "${_BOOST_URL}" CACHE STRING "URL of the boost archive.")
    set(BOOST_HASH "${_BOOST_HASH}" CACHE STRING "SHA256 hash of the boost archive.")
    include(dep_boost)

    if (BUILD_BLOSC)
      set(BLOSC_VERSION "1.5.0")
      set(BLOSC_URL "https://github.com/Blosc/c-blosc/archive/v${BLOSC_VERSION}.zip"
        CACHE STRING "URL of the c-BLOSC archive.")
      set(BLOSC_HASH "fddd00ae717fc840c49509c367444f37bc50daea7d7d431b6c05e9570bc443a9" 
        CACHE STRING "SHA256 hash of the c-BLOSC archive.")
      include(dep_blosc)
    endif()

    set(OPENVDB_VERSION "7.0.0")
    set(OPENVDB_BASE_URL "https://github.com/AcademySoftwareFoundation/openvdb/archive")
    set(OPENVDB_URL "${OPENVDB_BASE_URL}/v${OPENVDB_VERSION}.zip"
      CACHE STRING "URL of the OpenVDB archive.")
    set(OPENVDB_HASH "fe830cc99039fb0f82e55a1bc655d58e705b29efc7288130e8201de0b2930b90"
      CACHE STRING "SHA256 hash of the OpenVDB archive.")
    include(dep_openvdb)
  endif()
endif()

## ======================================================================== ##
## Build OpenVKL
## ======================================================================== ##

set(COMPONENT_NAME openvkl)

set(COMPONENT_PATH ${INSTALL_DIR_ABSOLUTE})
if (INSTALL_IN_SEPARATE_DIRECTORIES)
  set(COMPONENT_PATH ${INSTALL_DIR_ABSOLUTE}/${COMPONENT_NAME})
endif()

if (NOT BUILD_DEPENDENCIES_ONLY)
  ExternalProject_Add(openvkl
    PREFIX openvkl
    DOWNLOAD_COMMAND ""
    STAMP_DIR openvkl/stamp
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..
    BINARY_DIR openvkl/build
    LIST_SEPARATOR | # CMAKE_PREFIX_PATH uses alternate separator.
    CMAKE_ARGS
      -DCMAKE_PREFIX_PATH:PATH=${CMAKE_PREFIX_PATH}
      -DBUILD_TESTING=${BUILD_OPENVKL_TESTING}
      -DBUILD_BENCHMARKS=${BUILD_OPENVKL_BENCHMARKS}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      -DCMAKE_INSTALL_PREFIX=${COMPONENT_PATH}
      -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR}
      -DCMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR}
      -DCMAKE_INSTALL_DOCDIR=${CMAKE_INSTALL_DOCDIR}
      -DCMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}
      $<$<BOOL:${BUILD_ISPC}>:-DISPC_EXECUTABLE=${ISPC_PATH}>
      $<$<BOOL:${BUILD_TBB}>:-DRKCOMMON_TBB_ROOT=${TBB_PATH}>
      $<$<BOOL:${BUILD_OPENVDB}>:-DOpenVDB_ROOT=${OPENVDB_PATH}>
      ${OPENVKL_EXTRA_OPENVDB_OPTIONS}
    BUILD_COMMAND ${DEFAULT_BUILD_COMMAND}
    BUILD_ALWAYS OFF
  )

  ExternalProject_Add_StepDependencies(openvkl
  configure
    $<$<BOOL:${BUILD_ISPC}>:ispc>
    $<$<BOOL:${BUILD_TBB}>:tbb>
    $<$<BOOL:${BUILD_RKCOMMON}>:rkcommon>
    $<$<BOOL:${BUILD_GLFW}>:glfw>
    $<$<BOOL:${BUILD_EMBREE}>:embree>
    $<$<BOOL:${BUILD_OPENVDB}>:openvdb>
  )
endif()
