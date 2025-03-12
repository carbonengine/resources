cmake_minimum_required(VERSION 3.31)
project(carbon-resources VERSION 1.0.0)

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

find_package(yaml-cpp CONFIG REQUIRED) ## Can it be in tools?

# Add subdirectory for resource tools static library
add_subdirectory(tools)

set(SRC_FILES
    include/BinaryResourceGroup.h
    include/BundleResourceGroup.h
    include/Exports.h
    include/PatchResourceGroup.h
    include/ResourceGroup.h
    include/Enums.h
    
    src/BinaryResourceGroup.cpp
    src/BinaryResourceGroupImpl.cpp
    src/BinaryResourceGroupImpl.h
    src/BundleResourceGroup.cpp
    src/BundleResourceGroupImpl.cpp
    src/BundleResourceGroupImpl.h
    src/PatchResourceGroup.cpp
    src/PatchResourceGroupImpl.cpp
    src/PatchResourceGroupImpl.h
    src/ResourceGroup.cpp
    src/ResourceGroupImpl.cpp
    src/ResourceGroupImpl.h
    src/Macros.h
    
    src/ResourceInfo/BinaryResourceInfo.cpp
    src/ResourceInfo/BinaryResourceInfo.h
    src/ResourceInfo/BundleResourceInfo.cpp
    src/ResourceInfo/BundleResourceInfo.h
    src/ResourceInfo/PatchResourceInfo.cpp
    src/ResourceInfo/PatchResourceInfo.h
    src/ResourceInfo/ResourceInfo.cpp
    src/ResourceInfo/ResourceInfo.h
    src/ResourceInfo/ResourceGroupInfo.h
    src/ResourceInfo/ResourceGroupInfo.cpp
    src/ResourceInfo/BinaryResourceGroupInfo.h
    src/ResourceInfo/BinaryResourceGroupInfo.cpp
    src/ResourceInfo/PatchResourceGroupInfo.h
    src/ResourceInfo/PatchResourceGroupInfo.cpp
    src/ResourceInfo/BundleResourceGroupInfo.h
    src/ResourceInfo/BundleResourceGroupInfo.cpp
)

add_library(carbon-resources SHARED ${SRC_FILES})

target_compile_definitions(carbon-resources PRIVATE EXPORT_LIBRARY)

target_link_libraries(carbon-resources PRIVATE carbon-resources-tools yaml-cpp::yaml-cpp)

#target_include_directories(carbon-resources PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(carbon-resources
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

get_target_property(_SOURCES carbon-resources SOURCES)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        PREFIX "Sources"
        FILES ${_SOURCES}
)

set_target_properties(carbon-resources PROPERTIES OUTPUT_NAME "_carbon-resources")

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
        # Find Doxygen, which is needed to parse C++ source files
        find_package(Doxygen)

        set(DOCUMENTATION_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doc)

        # Evaluate config file for Doxygen to input project values
        set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in)
        set(DOXYFILE_OUT ${DOCUMENTATION_OUT_DIR}/Doxyfile)
        configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

        set(DOXYGEN_OUT_DIR ${DOCUMENTATION_OUT_DIR}/xml)
        set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUT_DIR}/index.xml)

        # Regenerate with source changes
        add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                DEPENDS ${_SOURCES}
                COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
                COMMENT "Running Doxygen"
                VERBATIM)

        add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

        # Set up sphinx build for Python autodoc comments.
        set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/doc/source)
        set(SPHINX_BUILD ${DOCUMENTATION_OUT_DIR}/build)

        # Select the correct pythonInterpreter to use based on build system.
        if(WIN32)
            set(CCP_PYTHON_INTERPRETER "${BRANCH_ROOT_DIR}/eve/client/pythonInterpreter.bat")
        elseif(APPLE)
            set(CCP_PYTHON_INTERPRETER "${BRANCH_ROOT_DIR}/eve/client/pythonInterpreter.sh")
        endif()

        # Run Sphinx against configuration file and Doxygen output
        add_custom_target(Sphinx ALL
                COMMAND ${CMAKE_COMMAND}
                -E env
                PYTHONPATH="$<TARGET_FILE_DIR:carbon-resources>\;${CMAKE_SOURCE_DIR}/python"
                BUILDFLAVOR=$<LOWER_CASE:$<CONFIG>>
                ${CCP_PYTHON_INTERPRETER}
                -m sphinx build -E -b html
                -Dbreathe_projects.doxygen=${DOXYGEN_OUT_DIR}
                -c ${SPHINX_SOURCE}
                ${SPHINX_SOURCE} ${SPHINX_BUILD}
                WORKING_DIRECTORY ${BRANCH_ROOT_DIR}
                DEPENDS ${DOXYGEN_INDEX_FILE}
                COMMENT "Generating documentation with Sphinx")

        # Install rule for documentation
        install(DIRECTORY ${SPHINX_BUILD} DESTINATION documentation)
    endif()

    set(INSTALL_BIN_ONLY OFF CACHE BOOL "Do not Install configuration files meant to be read by projects using this project as a dependency (Cmake Config/ header files etc . . .). Set this to ON when installing this component to the perforce vendor folder")
    if(INSTALL_BIN_ONLY)
        # Install rule to ensure that our runtime files are in the expected, platform-specific folder
        install(
                TARGETS carbon-resources
                EXPORT carbon-resourcesTarget
                CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
                RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        )
    else()
        # Install rule to ensure that our runtime and linker files are in the expected, platform-specific folders
        install(
                TARGETS carbon-resources
                EXPORT carbon-resourcesTarget
                CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
                LIBRARY DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
                ARCHIVE DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
                RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        )
        # Install rule for available public headers
        install(DIRECTORY include DESTINATION . FILES_MATCHING PATTERN "*.h")

        install(
                EXPORT carbon-resourcesTarget
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/carbon-resources
                FILE carbon-resources.cmake)
        install(
                FILES carbon-resourcesConfig.cmake
                DESTINATION ${CMAKE_INSTALL_PREFIX}/share/carbon-resources)
    endif()

    #if(WIN32)
    #    add_custom_command(
    #        TARGET carbon-resources POST_BUILD
    #        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:carbon-resources> $<TARGET_FILE_DIR:carbon-resources>
    #        COMMAND_EXPAND_LISTS
    #    )
    #elseif(APPLE)
    #        add_custom_command(
    #        TARGET carbon-resources POST_BUILD
    #        COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #        $<TARGET_FILE_DIR:ccp::log>/libCcpLog.dylib $<TARGET_FILE_DIR:carbon-resources>
    #        COMMAND_EXPAND_LISTS
    #)
    #endif()
endif()
