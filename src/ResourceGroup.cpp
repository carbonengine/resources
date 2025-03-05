#include "ResourceGroup.h"
#include "ResourceGroupImpl.h"
#include "Enums.h"

namespace CarbonResources
{

    ResourceGroup::ResourceGroup( ResourceGroupImpl* impl ) :
	    m_impl( impl )
    {
    }

    ResourceGroup::ResourceGroup( const std::string& relativePath ) :
	    m_impl( new ResourceGroupImpl( { relativePath } ) )
    {
    }

    ResourceGroup::~ResourceGroup()
    {
	    delete m_impl;
    }

    /// @brief Create bundle from resource group
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    /// @note will relinquish ownership of bundle resource group
    Result ResourceGroup::CreateBundle( const BundleCreateParams& params ) const
    {
	    return Result::FAIL;
    }

    /// @brief Create patch from resource group
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    /// @note will relinquish ownership of patch resource group
    Result ResourceGroup::CreatePatch( PatchCreateParams& params ) const
    {
	    return m_impl->CreatePatch( params );
    }

    /// @brief Import resource group to file
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    /// @note will relinquish ownership of patch resource group
    Result ResourceGroup::ImportFromFile( ResourceGroupImportFromFileParams& params ) const
    {
	    return m_impl->ImportFromFile( params );
    }

    /// @brief Export resource group to file
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    /// @note will relinquish ownership of patch resource group
    Result ResourceGroup::ExportToFile( const ResourceGroupExportToFileParams& params ) const
    {
	    return m_impl->ExportToFile( params );
    }

    /// @brief Return a subtraction
    /// @param data data which the checksum will be based on
    /// @param data_size size of data passed in
    /// @param checksum will contain the resulting checksum on success
    /// @return true on success, false on failure
    /// @note will relinquish ownership of patch resource group
    Result ResourceGroup::Subtraction( ResourceGroupSubtractionParams& params ) const
    {
	    return m_impl->Subtraction( params );
    }

    /*
    // TODO get this out of public API
    Result ResourceGroup::AddResource( Resource* r ) const
    {
		return m_impl->AddResource( r );
    }
    */

}