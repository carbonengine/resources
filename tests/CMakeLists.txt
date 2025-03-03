cmake_minimum_required(VERSION 3.18)

project(carbon-resources-test)

find_package(GTest CONFIG REQUIRED)
find_package(tiny-process-library CONFIG REQUIRED)

include(GoogleTest)

set(SRC_FILES
    src/CarbonResourcesTestFixture.cpp
    src/CarbonResourcesTestFixture.h
    src/CliTestFixture.cpp
    src/CliTestFixture.h
    src/CarbonResourcesLibraryTest.cpp
    src/CarbonResourcesCliTest.cpp
    src/ResourceToolsLibraryTest.cpp
)

add_executable(carbon-resources-test ${SRC_FILES})


file (GENERATE
    OUTPUT "Paths_$<CONFIG>.h"
    CONTENT
"
#pragma once

#define TEST_DATA_BASE_PATH \"${CMAKE_SOURCE_DIR}/tests/testData\"
#define CARBON_RESOURCES_CLI_EXE_NAME \"$<TARGET_FILE_BASE_NAME:carbon-resources-cli>.exe\"
"
)



add_custom_command(
        TARGET carbon-resources-test PRE_BUILD
        COMMAND ${CMAKE_COMMAND} "-E" "copy_if_different" "Paths_$<CONFIG>.h" "Paths.h"
        COMMAND_EXPAND_LISTS
    )

add_custom_target(GeneratePathsHeader DEPENDS "Paths.h")
add_dependencies(carbon-resources-test GeneratePathsHeader)

target_include_directories(carbon-resources-test PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(carbon-resources-test PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(carbon-resources-test PRIVATE GTest::gtest GTest::gtest_main tiny-process-library::tiny-process-library carbon-resources carbon-resources-tools)

if(WIN32)

    # Get CLI exe
    add_custom_command(
        TARGET carbon-resources-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:carbon-resources-cli> $<TARGET_FILE_DIR:carbon-resources-test>
        COMMAND_EXPAND_LISTS
    )

    # Binaries
    add_custom_command(
        TARGET carbon-resources-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:carbon-resources-test> $<TARGET_FILE_DIR:carbon-resources-test>
        COMMAND_EXPAND_LISTS
    )
elseif(APPLE)
  #TODO
        #add_custom_command(
        #TARGET carbon-resources-test POST_BUILD
        #COMMAND ${CMAKE_COMMAND} -E copy_if_different
        #$<TARGET_FILE_DIR:ccp::log>/libCcpLog.dylib $<TARGET_FILE_DIR:carbon-resources-test>
        #COMMAND_EXPAND_LISTS
#)
endif()

gtest_discover_tests(carbon-resources-test capiTests)
