cmake_minimum_required(VERSION 3.2)
project(sapt)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

option(BUILD_STATIC_LIBS "Build the static library (default is ON) " OFF)
option(BUILD_SHARED_LIBS "Build the shared library (default is OFF)" ON)

set(CMAKE_MACOSX_RPATH 1)

if (BUILD_STATIC_LIBS AND BUILD_SHARED_LIBS)
    MESSAGE(FATAL_ERROR " It is not allowed compiling both static and shared libraries and executables;
 - to enable/disable static compilation, set -DBUILD_STATIC_LIBS=[ON|OFF] (default is ON)
 - to enable/disable shared compilation, set -DBUILD_SHARED_LIBS=[ON|OFF] (default is OFF)")
endif ()

if (NOT BUILD_STATIC_LIBS AND NOT BUILD_SHARED_LIBS)
    MESSAGE(FATAL_ERROR " It is mandatory enabling either static or shared compilation of libraries and executables;
 - to enable/disable static compilation, set -DBUILD_STATIC_LIBS=[ON|OFF] (default is ON)
 - to enable/disable shared compilation, set -DBUILD_SHARED_LIBS=[ON|OFF] (default is OFF)")
endif ()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -O3 -Wall")

set(SOURCE_FILES
        suffixarray/dbkv.h
        suffixarray/sample.h
        suffixarray/CorpusStorage.cpp suffixarray/CorpusStorage.h
        suffixarray/UpdateBatch.cpp suffixarray/UpdateBatch.h
        suffixarray/PostingList.cpp suffixarray/PostingList.h
        suffixarray/PrefixCursor.cpp suffixarray/PrefixCursor.h
        suffixarray/SuffixArray.cpp suffixarray/SuffixArray.h
        suffixarray/Collector.cpp suffixarray/Collector.h

        sapt/Options.h
        sapt/TranslationOption.h
        sapt/PhraseTable.cpp sapt/PhraseTable.h
        sapt/UpdateManager.cpp sapt/UpdateManager.h

        util/hashutils.h
        util/ioutils.h
        util/chrono.h
        util/randutils.h util/randutils.cpp
        util/BilingualCorpus.cpp util/BilingualCorpus.h sapt/TranslationOptionBuilder.cpp sapt/TranslationOptionBuilder.h)

include_directories(${CMAKE_SOURCE_DIR})

add_library(${PROJECT_NAME} ${SOURCE_FILES})

file(GLOB executables executables/*.cpp)
foreach (exefile ${executables})
    get_filename_component(exe ${exefile} NAME_WE)
    add_executable(${exe} ${exefile})
    target_link_libraries(${exe} ${PROJECT_NAME})
endforeach ()

foreach (exefile ${executables})
    get_filename_component(exe ${exefile} NAME_WE)
    install(TARGETS ${exe}
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
            LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
            ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
endforeach ()

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)


install(FILES sapt/PhraseTable.h sapt/Options.h sapt/TranslationOption.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sapt)

## MMT common-interfaces
find_package(MMTCommons REQUIRED)
include_directories(${MMTCommons_INCLUDE_DIR})

## Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.55.0 REQUIRED COMPONENTS filesystem thread system program_options)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

## RocksDB
find_package(Rocksdb REQUIRED)
include_directories(${Rocksdb_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${Rocksdb_LIBS})

message(STATUS "Executables will be installed in ${CMAKE_INSTALL_PREFIX}/bin")
message(STATUS "Libraries will be installed in ${CMAKE_INSTALL_PREFIX}/lib")
message(STATUS "Include files will be installed in ${CMAKE_INSTALL_PREFIX}/include")

# OpenMP
find_package(OpenMP QUIET)
if (OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    message(STATUS "OpenMP found")
endif (OPENMP_FOUND)

# Test cases
add_subdirectory(test)
