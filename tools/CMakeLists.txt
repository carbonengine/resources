# Copyright © 2025 CCP ehf.

find_package(cryptopp CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(unofficial-inih CONFIG REQUIRED)

set(SRC_FILES
        include/BundleStreamIn.h
        include/BundleStreamOut.h
        include/ChunkIndex.h
        include/CompressedFileDataStreamOut.h
        include/Downloader.h
        include/FileDataStreamIn.h
        include/FileDataStreamOut.h
        include/FilterFileReader.h
        include/FilterIndexMappingFile.h
        include/GzipCompressionStream.h
        include/GzipDecompressionStream.h
        include/Md5ChecksumStream.h
        include/Patching.h
        include/ResourceFilter.h
        include/ResourceTools.h
        include/RollingChecksum.h
        include/ScopedFile.h
        include/ThreadPool.h

        src/BundleStreamIn.cpp
        src/BundleStreamOut.cpp
        src/ChunkIndex.cpp
        src/CompressedFileDataStreamOut.cpp
        src/Downloader.cpp
        src/FileDataStreamIn.cpp
        src/FileDataStreamOut.cpp
        src/FilterFileReader.cpp
        src/FilterIndexMappingFile.cpp
        src/GzipCompressionStream.cpp
        src/GzipDecompressionStream.cpp
        src/Md5ChecksumStream.cpp
        src/ResourceFilter.cpp
        src/ResourceTools.cpp
        src/ScopedFile.cpp
        src/Patching.cpp
        src/RollingChecksum.cpp
)

add_library(resources-tools STATIC ${SRC_FILES})

find_package(bsdiff CONFIG REQUIRED)

get_target_property(_SOURCES resources-tools SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        PREFIX "Sources"
        FILES ${_SOURCES}
)

if (WIN32)
    target_compile_definitions(resources-tools PRIVATE NOMINMAX) # Do not define min/max macros.
endif ()

target_link_libraries(resources-tools PRIVATE cryptopp::cryptopp CURL::libcurl ZLIB::ZLIB static_bsdiff unofficial::inih::inireader)

target_include_directories(resources-tools
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
