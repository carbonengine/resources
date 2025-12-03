// Copyright © 2025 CCP ehf.

#ifndef FILTERPREFIXMAP_H
#define FILTERPREFIXMAP_H

#include <string>
#include <map>
#include <vector>

namespace CarbonResources
{

class FilterPrefixmap
{
public:
	explicit FilterPrefixmap( const std::string& prefixmapStr );

	const std::map<std::string, std::vector<std::string>>& GetMap() const;

private:
	std::map<std::string, std::vector<std::string>> m_prefixMap;

	void Parse( const std::string& prefixmapStr );
};

}

#endif // FILTERPREFIXMAP_H
