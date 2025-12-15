// Copyright © 2025 CCP ehf.

#ifndef FILTERPREFIXMAPENTRY_H
#define FILTERPREFIXMAPENTRY_H

#include <string>
#include <set>

namespace ResourceTools
{

// Class representing all (one or more) path entries for a given prefix identifier.
class FilterPrefixMapEntry
{
public:
	explicit FilterPrefixMapEntry( const std::string& prefix, const std::string& rawPaths );

	// Parse rawPaths and appends to existing paths if needed.
	void AppendPaths( const std::string& prefix, const std::string& rawPaths );

	const std::string& GetPrefix() const;

	const std::set<std::string>& GetPaths() const;

private:
	std::string m_prefix;

	// The set of parsed paths (sorted).
	std::set<std::string> m_paths;
};

}

#endif // FILTERPREFIXMAPENTRY_H
