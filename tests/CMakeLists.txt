# Copyright © 2025 CCP ehf.

find_package(CURL CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(tiny-process-library CONFIG REQUIRED)

include(GoogleTest)

set(SRC_FILES
        src/ResourcesTestFixture.cpp
        src/ResourcesTestFixture.h
        src/CliTestFixture.cpp
        src/CliTestFixture.h
        src/ResourcesLibraryTest.cpp
        src/ResourcesCliTest.cpp
        src/ResourceToolsLibraryTest.cpp
)

add_executable(resources-test ${SRC_FILES})

target_compile_definitions(resources-test
        PRIVATE
        TEST_DATA_BASE_PATH="${CMAKE_SOURCE_DIR}/tests/testData"
        CARBON_RESOURCES_CLI_EXE_NAME=\"$<TARGET_FILE_BASE_NAME:resources-cli>\")

target_include_directories(resources-test PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

target_include_directories(resources-test PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(resources-test PRIVATE GTest::gtest GTest::gtest_main tiny-process-library::tiny-process-library resources resources-tools)

if (DEV_FEATURES)
    target_compile_definitions(resources-test PRIVATE DEV_FEATURES)
endif ()

if (WIN32)

    # Get CLI exe
    add_custom_command(
            TARGET resources-test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:resources-cli> $<TARGET_FILE_DIR:resources-test>
            COMMAND_EXPAND_LISTS
    )

elseif (APPLE)
    # Get CLI exe
    add_custom_command(
            TARGET resources-test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:resources-cli> $<TARGET_FILE_DIR:resources-test>
            COMMAND_EXPAND_LISTS

    )
endif ()

cmake_path(SET TEST_DATA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/testData)

gtest_discover_tests(
        resources-test capiTests
        PROPERTIES
        ENVIRONMENT "TEST_DATA_PATH=${TEST_DATA_PATH}"
        WORKING_DIRECTORY $<TARGET_FILE_DIR:resources-test>  # So that we can find the CLI executable.
        DISCOVERY_MODE PRE_TEST # Workaround to get builds working on macOS on ARM https://gitlab.kitware.com/cmake/cmake/-/issues/21845
)
