set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_ENV_PASSTHROUGH CCP_EVE_PERFORCE_BRANCH_PATH)
set(VCPKG_BUILD_TYPE release)

set(VCPKG_PLATFORM_TOOLSET v141)
set(VCPKG_CMAKE_SYSTEM_VERSION "10.0.17763.0")

# Verify at compile time that the port supports Windows 10.
set(VCPKG_C_FLAGS "-DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00")
set(VCPKG_CXX_FLAGS ${VCPKG_C_FLAGS})

if (PORT MATCHES "bsdiff-drake127")
    set(VCPKG_LIBRARY_LINKAGE static)
endif()

if (PORT MATCHES "yaml-cpp")
    set(VCPKG_LIBRARY_LINKAGE static)
endif()

if (PORT MATCHES "curl")
    set(VCPKG_LIBRARY_LINKAGE static)
endif()

if (PORT MATCHES "zlib")
    set(VCPKG_LIBRARY_LINKAGE static)
endif()