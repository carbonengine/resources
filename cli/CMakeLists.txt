cmake_minimum_required(VERSION 3.31)
project(carbon-resources-cli VERSION 1.0.0)

find_package(argparse CONFIG REQUIRED)

set(SRC_FILES
    src/main.cpp
)

ccp_add_executable(carbon-resources-cli ${SRC_FILES})

target_link_libraries(carbon-resources-cli PRIVATE carbon-resources argparse::argparse)

get_target_property(_SOURCES carbon-resources-cli SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        PREFIX "Sources"
        FILES ${_SOURCES}
)

set_target_properties(carbon-resources-cli PROPERTIES OUTPUT_NAME "carbon-resources")