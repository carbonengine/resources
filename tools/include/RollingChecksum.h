// Copyright © 2025 CCP ehf.

#pragma once

#include <cstdint>
#include <string>

namespace ResourceTools
{
struct RollingChecksum
{
	uint32_t alpha;
	uint32_t beta;
	uint32_t checksum;
};

// Generate a weak checksum using the rsync algorithm https://rsync.samba.org/tech_report/node3.html
RollingChecksum GenerateRollingAdlerChecksum( const std::string& input, uint32_t start, uint32_t end );

// Generate a weak checksum using the rsync algorithm https://rsync.samba.org/tech_report/node3.html
RollingChecksum GenerateRollingAdlerChecksum( const std::string& input, uint32_t start, uint32_t end, RollingChecksum previous );

}