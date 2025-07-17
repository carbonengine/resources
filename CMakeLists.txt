# Copyright © 2025 CCP ehf.

cmake_minimum_required(VERSION 3.31)
project(carbon-resources VERSION 2.0.0)

if(NOT DEFINED ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "Missing required environment variable CCP_EVE_PERFORCE_BRANCH_PATH")
elseif(NOT IS_ABSOLUTE $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH needs to be an absolute path")
elseif(NOT EXISTS $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH is pointing at a non-existing location $ENV{CCP_EVE_PERFORCE_BRANCH_PATH}")
elseif(NOT IS_DIRECTORY $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH needs to be a directory")
else()
    message(STATUS "Using CCP_EVE_PERFORCE_BRANCH_PATH located at $ENV{CCP_EVE_PERFORCE_BRANCH_PATH}")
endif()

file(TO_CMAKE_PATH "$ENV{CCP_EVE_PERFORCE_BRANCH_PATH}" BRANCH_ROOT_DIR)
list(INSERT CMAKE_MODULE_PATH 0 ${BRANCH_ROOT_DIR}/cmake)

include(cmake/CcpGlobalSettings.cmake)
include(cmake/CcpTargetConfigurations.cmake)
include(cmake/CcpDocsGenerator.cmake)

find_package(yaml-cpp CONFIG REQUIRED) ## Can it be in tools?
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Add subdirectory for resource tools static library
add_subdirectory(tools)

set(SRC_FILES
    include/BundleResourceGroup.h
    include/Exports.h
    include/PatchResourceGroup.h
    include/ResourceGroup.h
    include/Enums.h
    include/Version.h

    src/BundleResourceGroup.cpp
    src/BundleResourceGroupImpl.cpp
    src/BundleResourceGroupImpl.h
    src/PatchResourceGroup.cpp
    src/PatchResourceGroupImpl.cpp
    src/PatchResourceGroupImpl.h
    src/ResourceGroup.cpp
    src/ResourceGroupFactory.h
    src/ResourceGroupFactory.cpp
    src/ResourceGroupImpl.cpp
    src/ResourceGroupImpl.h
    src/Enums.cpp
    src/VersionInternal.h
    src/VersionInternal.cpp

    src/ResourceInfo/BundleResourceInfo.cpp
    src/ResourceInfo/BundleResourceInfo.h
    src/ResourceInfo/PatchResourceInfo.cpp
    src/ResourceInfo/PatchResourceInfo.h
    src/ResourceInfo/ResourceInfo.cpp
    src/ResourceInfo/ResourceInfo.h
    src/ResourceInfo/ResourceGroupInfo.h
    src/ResourceInfo/ResourceGroupInfo.cpp
    src/ResourceInfo/PatchResourceGroupInfo.h
    src/ResourceInfo/PatchResourceGroupInfo.cpp
    src/ResourceInfo/BundleResourceGroupInfo.h
    src/ResourceInfo/BundleResourceGroupInfo.cpp
    src/ParameterVersion.h
    src/ParameterVersion.cpp
)

ccp_add_library(carbon-resources SHARED ${SRC_FILES})

target_compile_definitions(carbon-resources PRIVATE EXPORT_LIBRARY)

# Version is required in code
# Added as config
configure_file(src/Version.h.in include/Version.h @ONLY)

if(WIN32)
    target_compile_definitions(carbon-resources PRIVATE NOMINMAX) # Do not define min/max macros.
endif()

target_link_libraries(carbon-resources PRIVATE carbon-resources-tools yaml-cpp::yaml-cpp)

target_include_directories(carbon-resources
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

get_target_property(_SOURCES carbon-resources SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        PREFIX "Sources"
        FILES ${_SOURCES}
)

set_target_properties(carbon-resources PROPERTIES OUTPUT_NAME "_carbon-resources")

# Static lib, exported and used for cli
ccp_add_library(carbon-resources-static STATIC ${SRC_FILES})

set_target_properties(carbon-resources-static PROPERTIES OUTPUT_NAME "_carbon-resources-static")

target_compile_definitions(carbon-resources-static PUBLIC CARBON_RESOURCES_STATIC)

if(WIN32)
    target_compile_definitions(carbon-resources-static PRIVATE NOMINMAX) # Do not define min/max macros.
endif()

target_link_libraries(carbon-resources-static PRIVATE $<BUILD_LOCAL_INTERFACE:carbon-resources-tools> yaml-cpp::yaml-cpp)

target_include_directories(carbon-resources-static
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

add_subdirectory(cli)

enable_testing()

add_subdirectory(tests)


# Provide an install target if this is the top level project only
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Determine if/when to generate/build documentation. The rules are as follows:
    # - If running locally - Build documentation is OFF by default
    # - If running on a build agent (TeamCity) - Build documentation is ON by default
    # - If -DBUILD_DOCUMENTATION=ON/OFF flag is explicitly set, use it.
    set(BUILD_DOCUMENTATION_DEFAULT_FLAG OFF)
    if(DEFINED ENV{TEAMCITY_VERSION})
        set(BUILD_DOCUMENTATION_DEFAULT_FLAG ON)
    endif()
    message(STATUS "Document generation settings: default=${BUILD_DOCUMENTATION_DEFAULT_FLAG}, option BUILD_DOCUMENTATION=${BUILD_DOCUMENTATION}")
    option(BUILD_DOCUMENTATION "Build Documentation" ${BUILD_DOCUMENTATION_DEFAULT_FLAG})

    if(BUILD_DOCUMENTATION)
        # Run sphinx
        set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/doc/source)
        set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/doc/build)

        create_carbon_docs_sphinx_target(
                PYTHON_EXE ${Python3_EXECUTABLE}
                VENV_NAME docs_generation_venv
                PYTHONPATH_ENV $ENV{PYTHONPATH}
                SPHINX_SOURCE ${SPHINX_SOURCE}
                SPHINX_BUILD ${SPHINX_BUILD}
                DOXYGEN_SRC_FILES ${SRC_FILES}
                SPHINX_TARGET_NAME Sphinx
                DOXYGEN_TARGET_NAME Doxygen
                INSTALL_DESTINATION documentation
        )

        # Ensure that carbon-resources is built before Spinx target
        add_dependencies(Sphinx carbon-resources)
    endif()

    set(INSTALL_BIN_ONLY OFF CACHE BOOL "Do not Install configuration files meant to be read by projects using this project as a dependency (Cmake Config/ header files etc . . .). Set this to ON when installing this component to the perforce vendor folder")
    if(INSTALL_BIN_ONLY)
        # Install rule to ensure that our runtime files are in the expected, platform-specific folder
        install(
                TARGETS carbon-resources carbon-resources-static
                EXPORT carbon-resourcesTarget
                CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
                LIBRARY DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
                ARCHIVE DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
                RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        )
    else()
        # Install rule to ensure that our runtime and linker files are in the expected, platform-specific folders
        install(
                TARGETS carbon-resources carbon-resources-static
                EXPORT carbon-resourcesTarget
                CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
                LIBRARY DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
                ARCHIVE DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
                RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        )
        # Install rule for available public headers
        install(DIRECTORY include DESTINATION . FILES_MATCHING PATTERN "*.h")

        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include DESTINATION . FILES_MATCHING PATTERN "*.h")

        install(
                EXPORT carbon-resourcesTarget
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/carbon-resources
                FILE carbon-resources.cmake)
        install(
                FILES carbon-resourcesConfig.cmake
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/carbon-resources)
    endif()

endif()
