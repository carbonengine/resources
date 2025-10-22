# Copyright © 2025 CCP ehf.

project(resources-cli VERSION 1.0.0)

find_package(argparse CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

set(SRC_FILES
        src/ApplyPatchCliOperation.h
        src/ApplyPatchCliOperation.cpp
        src/Cli.cpp
        src/Cli.h
        src/CliOperation.cpp
        src/CliOperation.h
        src/CreateBundleCliOperation.cpp
        src/CreateBundleCliOperation.h
        src/CreatePatchCliOperation.cpp
        src/CreatePatchCliOperation.h
        src/CreateResourceGroupCliOperation.cpp
        src/CreateResourceGroupCliOperation.h
        src/Defines.h
        src/DiffResourceGroupCliOperation.cpp
        src/DiffResourceGroupCliOperation.h
        src/main.cpp
        src/MergeResourceGroupCliOperation.cpp
        src/MergeResourceGroupCliOperation.h
        src/RemoveResourcesCliOperation.cpp
        src/RemoveResourcesCliOperation.h
        src/UnpackBundleCliOperation.cpp
        src/UnpackBundleCliOperation.h
)

ccp_add_executable(resources-cli ${SRC_FILES})

target_link_libraries(resources-cli PRIVATE resources-static argparse::argparse yaml-cpp::yaml-cpp)

get_target_property(_SOURCES resources-cli SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        PREFIX "Sources"
        FILES ${_SOURCES}
)

option(DEV_FEATURES "Expose unsupported features")

if (DEV_FEATURES)
    target_compile_definitions(resources-cli PRIVATE DEV_FEATURES)
endif ()

set_target_properties(resources-cli PROPERTIES OUTPUT_NAME "resources")

# Install rule to ensure that our runtime files are in the expected, platform-specific folder
install(
        TARGETS resources-cli
        EXPORT resources-cliTarget
        CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
        RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
)