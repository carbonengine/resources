cmake_minimum_required(VERSION 3.31)
project(carbon-resources-tools VERSION 1.0.0)

find_package(cryptopp CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)

set(SRC_FILES
    include/ResourceTools.h
    src/ResourceTools.cpp
)

add_library(carbon-resources-tools STATIC ${SRC_FILES})

#target_link_libraries(carbon-resources-cli P)

get_target_property(_SOURCES carbon-resources-tools SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        PREFIX "Sources"
        FILES ${_SOURCES}
)

target_link_libraries(carbon-resources-tools PRIVATE cryptopp::cryptopp CURL::libcurl)

target_include_directories(carbon-resources-tools PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
