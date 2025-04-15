cmake_minimum_required(VERSION 3.18)

project(carbon-resources-test)

find_package(CURL CONFIG REQUIRED)
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

target_compile_definitions(carbon-resources-test
        PRIVATE
        TEST_DATA_BASE_PATH="${CMAKE_SOURCE_DIR}/tests/testData"
        CARBON_RESOURCES_CLI_EXE_NAME=\"$<TARGET_FILE_BASE_NAME:carbon-resources-cli>.exe\")

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

    # Curl depends on zlib1.dll, which needs to be copied explicitly.
    add_custom_command(
        TARGET carbon-resources-test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:CURL::libcurl_shared>/zlib1.dll $<TARGET_FILE_DIR:carbon-resources-test>
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

cmake_path(SET TEST_DATA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/testData)

gtest_discover_tests(
        carbon-resources-test capiTests
        PROPERTIES
            ENVIRONMENT "TEST_DATA_PATH=${TEST_DATA_PATH}"
)
