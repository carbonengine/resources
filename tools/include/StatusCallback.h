#pragma once
#include <functional>
#include <string>

namespace ResourceTools
{
// This status callback should match the one in the CarbonResources namespace.
// We want to be able to just pass these into the tools in order to allow them
// to provide status updates.
using StatusCallback = std::function<void( int, int, const std::string& )>;
}