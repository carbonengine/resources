cmake_minimum_required(VERSION 3.31)
project(carbon-resources-tools VERSION 1.0.0)

find_package(cryptopp CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

set(SRC_FILES
    include/BundleStreamIn.h
    include/BundleStreamOut.h
    include/FileDataStreamIn.h
    include/FileDataStreamOut.h
    include/Md5ChecksumStream.h
    include/Patching.h
    include/ResourceTools.h
    include/ScopedFile.h
    
    src/BundleStreamIn.cpp
    src/BundleStreamOut.cpp
    src/FileDataStreamIn.cpp
    src/FileDataStreamOut.cpp
    src/Md5ChecksumStream.cpp
    src/ResourceTools.cpp
    src/ScopedFile.cpp
    src/Patching.cpp
)

add_library(carbon-resources-tools STATIC ${SRC_FILES})

#target_link_libraries(carbon-resources-cli P)
find_package(bsdiff CONFIG REQUIRED)

get_target_property(_SOURCES carbon-resources-tools SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        PREFIX "Sources"
        FILES ${_SOURCES}
)

target_link_libraries(carbon-resources-tools PRIVATE cryptopp::cryptopp CURL::libcurl ZLIB::ZLIB static_bsdiff)

target_include_directories(carbon-resources-tools PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
