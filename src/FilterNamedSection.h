// Copyright © 2025 CCP ehf.

#ifndef FILTERNAMEDSECTION_H
#define FILTERNAMEDSECTION_H

#include <string>
#include <vector>
#include "FilterResourceFilter.h"
#include "FilterResourceLine.h"

namespace CarbonResources
{

class FilterNamedSection
{
public:
	explicit FilterNamedSection( const std::string& filter, const std::string& resfile, const std::vector<std::string>& respaths );

private:
	FilterResourceFilter m_filter;
	std::string m_resfile;
	std::vector<std::string> m_respaths;
	// Optionally, store parsed FilterResourceLine objects
};

}

#endif // FILTERNAMEDSECTION_H
