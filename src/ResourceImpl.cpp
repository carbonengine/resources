#include "ResourceImpl.h"

#include <sstream>

#include <ResourceTools.h>

namespace CarbonResources
{

    Resource::ResourceImpl::ResourceImpl( const ResourceParams& params ):
	    m_resourceParameters(params)
    {
		
    }

    Resource::ResourceImpl::~ResourceImpl()
    {

    }

    ResourceParams& Resource::ResourceImpl::GetResourceParams()
    {
		return m_resourceParameters;
    }

    Result Resource::ResourceImpl::GetData( const ResourceGetDataParams& params )
    {
        // Construct path
		std::stringstream ss;

		ss << params.basePath;

        ss << m_resourceParameters.relativePath.GetValue();

        std::string path = ss.str();

		ResourceTools::GetFileData( path, params.data );

		return Result::SUCCESS;
    }

}