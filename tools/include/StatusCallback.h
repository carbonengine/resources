#pragma once
#include <functional>
#include <string>

namespace ResourceTools
{
// The status callback should be called with a percentage value and a message.
using StatusCallback = std::function<void( unsigned int, const std::string& )>;
}