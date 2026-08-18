# Copyright © 2025 CCP ehf.

cmake_minimum_required(VERSION 3.31.1)

option(BUILD_CLI "Build CLI." OFF)
if (BUILD_CLI)
    list(APPEND VCPKG_MANIFEST_FEATURES "cli")
endif()

option(BUILD_DOCUMENTATION "Build Documentation." OFF)
if (BUILD_DOCUMENTATION)
    list(APPEND VCPKG_MANIFEST_FEATURES "docs")
endif()

option(BUILD_TESTING "Build and run tests." OFF)
if (BUILD_TESTING)
    list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

project(resources VERSION 5.0.0)

include(cmake/CcpTargetConfigurations.cmake)
include(cmake/CcpDocsGenerator.cmake)
include(cmake/CcpPackageConfigHelpers.cmake)
include(cmake/CcpBuildConfigurations.cmake)

find_package(yaml-cpp CONFIG REQUIRED)

# Add subdirectory for resource tools static library
add_subdirectory(tools)

set(PUBLIC_HEADER_FILES
    include/BundleResourceGroup.h
    include/Exports.h
    include/PatchResourceGroup.h
    include/ResourceGroup.h
    include/Enums.h
)

# Version is required in code
# Added as config
configure_file(src/Version.h.in include/Version.h @ONLY)

set(PUBLIC_GENERATED_VERSION_HEADER_FILE include/Version.h)

set(SRC_FILES
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
    src/StatusSettings.h
    src/StatusSettings.cpp
    src/ParameterVersion.h
    src/ParameterVersion.cpp
)

add_library(resources STATIC ${SRC_FILES} ${PUBLIC_HEADER_FILES} ${PUBLIC_GENERATED_VERSION_HEADER_FILE})

target_compile_definitions(resources PRIVATE EXPORT_LIBRARY)

if (WIN32)
    target_compile_definitions(resources PRIVATE NOMINMAX) # Do not define min/max macros.
endif ()

target_compile_definitions(resources PUBLIC CARBON_RESOURCES_STATIC)

target_link_libraries(resources PRIVATE $<BUILD_LOCAL_INTERFACE:resources-tools> yaml-cpp::yaml-cpp)

target_include_directories(resources
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

get_target_property(_SOURCES resources SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        PREFIX "Sources"
        FILES ${_SOURCES}
)

if(BUILD_CLI)
    add_subdirectory(cli)
endif()

# Enable testing
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if (BUILD_TESTING)
        include(CTest)
        add_subdirectory(tests)
    endif ()
endif ()

# Provide an install target if this is the top level project only
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(INSTALL_TO_MONOLITH "Install this package to a perforce branch vendor folder - provided for backwards compatability" OFF)

    if (BUILD_DOCUMENTATION)
        find_package(Python3 COMPONENTS Interpreter REQUIRED)

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

        # Ensure that resources is built before Spinx target
        add_dependencies(Sphinx resources)
    endif ()

    # Install rule for available public headers
    install(FILES ${PUBLIC_HEADER_FILES} DESTINATION include)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PUBLIC_GENERATED_VERSION_HEADER_FILE} DESTINATION include)

    if(INSTALL_TO_MONOLITH)
        # Install rule to ensure that our runtime files are in the expected, platform-specific folder
        install(
                TARGETS resources
                EXPORT resourcesTarget
                CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
                LIBRARY DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
                ARCHIVE DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
                RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        )
        install(
                TARGETS resources-tools
                EXPORT resourcesToolsTarget
                CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
                LIBRARY DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
                ARCHIVE DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
                RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        )
    else()
        install(
            TARGETS resources
            EXPORT ResourcesTargets
            CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
            LIBRARY DESTINATION lib/
            ARCHIVE DESTINATION lib/
            RUNTIME DESTINATION bin/
        )

        install(
            TARGETS resources-tools
            EXPORT ResourcesTargets
            CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
            LIBRARY DESTINATION lib/
            ARCHIVE DESTINATION lib/
            RUNTIME DESTINATION bin/
        )

        install(
            EXPORT ResourcesTargets
            DESTINATION share/carbon-resources
            FILE carbon-resourcesConfig.cmake
        )
    endif()

endif ()
