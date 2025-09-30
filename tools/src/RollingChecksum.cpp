// Copyright © 2025 CCP ehf.

#include "RollingChecksum.h"

constexpr uint32_t ROLLING_CHECKSUM_MODULO{ 2 << 15 };

namespace ResourceTools
{
RollingChecksum GenerateRollingAdlerChecksum( const std::string& input, uint32_t start, uint32_t end )
{
	uint32_t alpha = 0;
	auto substring = input.substr( start, end - start );
	for( auto c : substring )
	{
		alpha += c;
	}
	alpha %= ROLLING_CHECKSUM_MODULO;

	uint32_t beta = 0;
	for( uint32_t i = start; i < end; ++i )
	{
		beta += ( end - i ) * substring[i - start];
	}
	beta %= ROLLING_CHECKSUM_MODULO;

	RollingChecksum rc;
	rc.alpha = alpha;
	rc.beta = beta;
	rc.checksum = alpha + ( beta * ROLLING_CHECKSUM_MODULO );

	return rc;
}

RollingChecksum GenerateRollingAdlerChecksum( const std::string& input, uint32_t start, uint32_t end, RollingChecksum previous )
{
	uint32_t alpha = previous.alpha - input[start - 1] + input[end - 1];
	alpha %= ROLLING_CHECKSUM_MODULO;

	uint32_t beta = previous.beta + alpha - ( end - start ) * input[start - 1];
	beta %= ROLLING_CHECKSUM_MODULO;

	RollingChecksum rc;
	rc.alpha = alpha;
	rc.beta = beta;
	rc.checksum = alpha + ( beta * ROLLING_CHECKSUM_MODULO );

	return rc;
}


}