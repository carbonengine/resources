/* 
	*************************************************************************

	ResourceInfo.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef ResourceInfo_H
#define ResourceInfo_H

#include <string>
#include <sstream>
#include <optional>
#include <vector>
#include <filesystem>
#include<yaml-cpp/yaml.h>
#include "Enums.h"
#include "ResourceGroup.h"
#include "../VersionInternal.h"
#include "../ParameterVersion.h"

namespace ResourceTools
{
    class FileDataStreamIn;
    class FileDataStreamOut;
}

namespace CarbonResources
{
	class VersionedParameter
	{
	public:
		VersionedParameter( Parameter parameter, std::string context ) : m_parameter( parameter ), m_context(std::move( context ))
		{
			const ParameterInfo* info = GetParameterInfo( parameter );
			m_tag = info->m_tag;
		};

		bool IsParameterExpectedInDocumentVersion( VersionInternal documentVersion ) const
		{
			return IsParameterExpected( m_parameter, m_context, documentVersion );
		}

	    std::string GetTag() const
	    {
		    return m_tag;
	    }

	protected:
	    std::string m_tag;
		std::string m_context;
		Parameter m_parameter;
	};

    template <typename T>
    class DocumentParameter : public VersionedParameter
    {
    public:
	    DocumentParameter( Parameter parameter, const std::string& context ) :
		    VersionedParameter( parameter, context )
	    {
	    }

	    void operator=( T value )
	    {
		    m_value = value;
	    }

	    T GetValue() const
	    {
		    return m_value.value();
	    }

	    bool HasValue() const
	    {
            
		    return m_value.has_value();
	    }

        void Reset()
        {
			m_value.reset();
        }

    protected:
	    std::optional<T> m_value;
    };

    template <typename T>
    class DocumentParameterCollection : public DocumentParameter<std::vector<T>*>
    {
    public:
	    DocumentParameterCollection(Parameter parameter, const std::string& context ) :
		    DocumentParameter<std::vector<T>*>( parameter, context )
	    {
		    this->m_value = &m_collection;
	    }

	    void PushBack( T attribute )
	    {
		    m_collection.push_back( attribute );
	    }

        void Clear()
        {
            m_collection.clear();
        }

        size_t GetSize() const
        {
			return m_collection.size();
        }

        T At(unsigned int index)
		{
			return m_collection.at( index );
		}

        typename std::vector<T>::iterator Find( const T other )
		{
            for (auto iter = m_collection.begin(); iter != m_collection.end(); iter++)
            {
				T attribute = (*iter);

                if( attribute->operator==( other ) ) // This is assuming pointers again, not allowed
				{
					return iter;
				}
            }

            return m_collection.end();
		}

        bool Contains( const T other )
		{
            for (T attribute : m_collection)
            {
				if( attribute->operator==(other) )  // This is assuming pointers again, not allowed
                {
					return true;
                }
            }

			return false;
        }

        void Erase( typename std::vector<T>::const_iterator attributeIterator )
		{
			m_collection.erase( attributeIterator );
		}

	    void Remove( typename std::vector<T>::const_iterator attributeIterator )
	    {
			delete( *attributeIterator );
			
		    m_collection.erase( attributeIterator );
	    }

	    typename std::vector<T>::iterator begin()
	    {
		    return m_collection.begin();
	    }

	    typename std::vector<T>::const_iterator begin() const
	    {
		    return m_collection.begin();
	    }

	    typename std::vector<T>::const_iterator cbegin()
	    {
		    return m_collection.begin();
	    }

	    typename std::vector<T>::iterator end()
	    {
		    return m_collection.end();
	    }

	    typename std::vector<T>::const_iterator end() const
	    {
		    return m_collection.end();
	    }

	    typename std::vector<T>::const_iterator cend()
	    {
		    return m_collection.end();
	    }

    protected:
	    std::vector<T> m_collection;
    };

    class Location
	{
	public:
		Location()
		{
		}

		Location( const std::string inLocation ) :
			location( inLocation )
		{
		}

		Result SetFromRelativePathAndDataChecksum( const std::filesystem::path& relativePath, const std::string& dataChecksum );

        std::string ToString()
        {
			return location;
        }

    private:

        std::string CalculateLocationFromChecksums( const std::string& relativePathChecksum, const std::string& dataChecksum ) const;

	private:

		std::string location = "";
	};


    struct ResourceInfoParams
	{
		std::filesystem::path relativePath;

		std::string location = "";

		std::string checksum = "";

		uintmax_t compressedSize = 0;

		uintmax_t uncompressedSize = 0;

    	unsigned int binaryOperation = 0;

    	std::string prefix;
	};

    struct ResourceGetDataStreamParams
	{
		ResourceSourceSettings resourceSourceSettings;

		std::shared_ptr<ResourceTools::FileDataStreamIn> dataStream;

        std::filesystem::path cacheBasePath = "cache";

		std::string expectedChecksum = "";

    	std::chrono::seconds downloadRetrySeconds{120};
	};

    struct ResourceGetDataParams
	{
		ResourceSourceSettings resourceSourceSettings;

		std::string* data = nullptr;

        std::filesystem::path cacheBasePath = "cache";

        std::string expectedChecksum = "";

    	std::chrono::seconds downloadRetrySeconds{120};
	};

    struct ResourcePutDataStreamParams
	{
		ResourceDestinationSettings resourceDestinationSettings;

		ResourceTools::FileDataStreamOut* dataStream = nullptr;
	};

    struct ResourcePutDataParams
	{
		ResourceDestinationSettings resourceDestinationSettings;

		std::string* data = nullptr;
	};


    class ResourceInfo
    {
    public:
	    ResourceInfo( const ResourceInfoParams& params );

	    virtual ~ResourceInfo();

	    void SetRelativePath( const std::filesystem::path& relativePath );

    	Result GetBinaryOperation( unsigned int& binaryOperation ) const;

		Result GetRelativePath(std::filesystem::path& relativePath) const;

	    Result GetLocation(std::string& location) const;

        Result GetType(std::string& type) const;

	    Result GetChecksum(std::string& checksum) const;

	    Result GetUncompressedSize( uintmax_t& uncompressedSize ) const;

	    Result GetCompressedSize( uintmax_t& compressedSize ) const;

        Result GetDataStream( ResourceGetDataStreamParams& params ) const;

	    Result GetData( ResourceGetDataParams& params ) const;

        Result PutDataStream( ResourcePutDataStreamParams& params ) const;

        Result PutData( ResourcePutDataParams& params ) const;

        virtual Result ImportFromYaml( YAML::Node& resource, const VersionInternal& documentVersion ); 

        virtual Result ExportToYaml( YAML::Emitter& out, const VersionInternal& documentVersion );

    	Result ExportToCsv( std::string& out, const VersionInternal& documentVersion );

        Result SetParametersFromData( const std::string& data );

    	Result SetParametersFromSourceStream( ResourceTools::FileDataStreamIn& stream, size_t matchSize );

		void SetDataChecksum( const std::string& checksum );

    	void SetCompressedSize( uintmax_t compressedSize );

    	void SetUncompressedSize( uintmax_t uncompressedSize );

        virtual Result SetParametersFromResource( const ResourceInfo* other, const VersionInternal& documentVersion );

        bool operator==( const ResourceInfo* other ) const;

        bool operator<( const ResourceInfo& other ) const;

        static std::string TypeId();

    private:
	    Result GetDataLocalRelative( ResourceGetDataParams& params, const int basePathId ) const;

	    Result GetDataLocalCdn( ResourceGetDataParams& params, const int basePathId ) const;

	    Result GetDataRemoteCdn( ResourceGetDataParams& params, const int basePathId ) const;

        Result GetDataStreamLocalRelative( ResourceGetDataStreamParams& params, const int basePathId ) const;

		Result GetDataStreamLocalCdn( ResourceGetDataStreamParams& params, const int basePathId ) const;

		Result GetDataStreamRemoteCdn( ResourceGetDataStreamParams& params, const int basePathId ) const;

        Result PutDataLocalRelative( ResourcePutDataParams& params ) const;

		Result PutDataLocalCdn( ResourcePutDataParams& params ) const;

        Result PutDataStreamLocalRelative( ResourcePutDataStreamParams& params ) const;

		Result PutDataStreamLocalCdn( ResourcePutDataStreamParams& params ) const;

        Result PutDataStreamRemoteCdn( ResourcePutDataStreamParams& params ) const;

        Result PutDataRemoteCdn( ResourcePutDataParams& params ) const;

    	Result UpdateLocation(); // Regenerate location parameter after changing checksum or relative path.

    protected:

        // Parameters for document version 0.0.0
		DocumentParameter<std::filesystem::path> m_relativePath = DocumentParameter<std::filesystem::path>( RELATIVE_PATH, TypeId() );

		DocumentParameter<Location> m_location = DocumentParameter<Location>( LOCATION, TypeId() );

        DocumentParameter<std::string> m_type = DocumentParameter<std::string>( TYPE, TypeId() );

		DocumentParameter<std::string> m_checksum = DocumentParameter<std::string>( CHECKSUM, TypeId() );

		DocumentParameter<uintmax_t> m_compressedSize = DocumentParameter<uintmax_t>( COMPRESSED_SIZE, TypeId() );

		DocumentParameter<uintmax_t> m_uncompressedSize = DocumentParameter<uintmax_t>( UNCOMPRESSED_SIZE, TypeId() );

    	DocumentParameter<unsigned int> m_binaryOperation = DocumentParameter<unsigned int>( BINARY_OPERATION, TypeId() );

    	DocumentParameter<std::string> m_prefix = DocumentParameter<std::string>( PREFIX, TypeId() );
    };

	inline Result SetParameterFromYamlNodeData(YAML::Node& node, DocumentParameter<std::filesystem::path>& parameter)
    {
	    parameter = node.as<std::string>();
		return Result{ ResultType::SUCCESS };
    }

	template <typename T>
	Result SetParameterFromYamlNodeData(YAML::Node& node, DocumentParameter<T>& parameter)
    {
	    parameter = node.as<T>();
		return Result{ ResultType::SUCCESS };
    }

	template <typename T>
	Result SetParameterFromYamlNode(YAML::Node& resource, DocumentParameter<T>& parameter, const std::string& context, const VersionInternal& documentVersion )
    {
		if( !parameter.IsParameterExpectedInDocumentVersion( documentVersion ) )
		{
			return Result{ ResultType::SUCCESS }; // Ignore unexpected parameter
		}
		YAML::Node param = resource[parameter.GetTag()];
		if( !param.IsDefined() )
		{
			return Result{ ResultType::MALFORMED_RESOURCE_INPUT };
		}
		return SetParameterFromYamlNodeData( param, parameter );
    }

}
#endif // ResourceInfo_H