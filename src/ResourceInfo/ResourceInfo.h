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
#include "Enums.h"
#include "ResourceGroup.h"

namespace YAML
{
    class Emitter;
    class Node;
}

namespace CarbonResources
{
    

    template <typename T>
    class DocumentParameter
    {
    public:
	    DocumentParameter( Version version, const std::string& tag ) :
		    m_version( version ),
		    m_tag( tag )
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

	    Version GetVersionTag()
	    {
		    return m_version;
	    }

	    bool IsParameterExpectedInDocumentVersion( Version documentVersion ) const
	    {
		    return documentVersion >= m_version;
	    }

	    std::string GetTag() const
	    {
		    return m_tag;
	    }

    protected:
	    Version m_version;
	    std::optional<T> m_value;
	    std::string m_tag;
    };

    template <typename T>
    class DocumentParameterCollection : public DocumentParameter<std::vector<T>*>
    {
    public:
	    DocumentParameterCollection( Version version, const std::string& tag ) :
		    DocumentParameter<std::vector<T>*>( version, tag )
	    {
		    this->m_value = &m_collection;
	    }

	    void PushBack( T attribute )
	    {
		    m_collection.push_back( attribute );
	    }

        void Clear()
        {
            for (T attribute : m_collection)
            {
				delete attribute;   //TODO T might not be a pointer
            }

            m_collection.clear();
        }

        size_t GetSize()
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

	    void Remove( typename std::vector<T>::const_iterator attributeIterator )
	    {
			delete ( *attributeIterator );

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

		Result SetFromRelativePathAndDataChecksum( const std::string& resourceType, const std::filesystem::path& relativePath, const std::string& dataChecksum );

        std::string ToString()
        {
			return location;
        }

	private:

		std::string location = "";
	};


    struct ResourceInfoParams
	{
		std::filesystem::path relativePath;

		std::string location = "";

		std::string checksum = "";

		unsigned long compressedSize = 0;

		unsigned long uncompressedSize = 0;

		unsigned long something = 0;
	};

    struct ResourceGetDataParams
	{
		ResourceSourceSettings resourceSourceSettings;

		std::string* data = nullptr;
	};

    struct ResourcePutDataParams
	{
		ResourceDestinationSettings resourceDestinationSettings;

		std::string* data = nullptr;
	};

	class ResourcePutDataParams;


    class ResourceInfo
    {
    public:
	    ResourceInfo( const ResourceInfoParams& params );

	    ~ResourceInfo();

	    void SetRelativePath( const std::filesystem::path& relativePath );

         // TODO Convert to return Result as they may not have a value
		// Also keeps returns the same
		Result GetRelativePath(std::filesystem::path& relativePath) const;

	    Result GetLocation(std::string& location) const;

        Result GetType(std::string& type) const;

	    Result GetChecksum(std::string& checksum) const;

	    Result GetUncompressedSize(unsigned long& uncompressedSize) const;

	    Result GetCompressedSize(unsigned long& compressedSize) const;

	    Result GetSomething(unsigned long& something) const;

	    Result GetData( ResourceGetDataParams& params ) const;

        Result PutData( ResourcePutDataParams& params ) const;

        virtual Result ImportFromYaml( YAML::Node& resource, const Version& documentVersion ); 

        virtual Result ExportToYaml( YAML::Emitter& out, const Version& documentVersion );

        Result SetParametersFromData( const std::string& data );

        Result SetParametersFromResource( const ResourceInfo* other );

        bool operator==( const ResourceInfo* other ) const 
		{
            if (!m_relativePath.HasValue())
            {
				return false;
            }
            if (!other->m_relativePath.HasValue())
            {
				return false;
            }
            // Equality is defined as having the same relative path, not same data
			return ( m_relativePath.GetValue() == other->m_relativePath.GetValue() );
		}

        static std::string TypeId();

    private:
	    Result GetDataLocalRelative( ResourceGetDataParams& params ) const;

	    Result GetDataLocalCdn( ResourceGetDataParams& params ) const;

	    Result GetDataRemoteCdn( ResourceGetDataParams& params ) const;

        Result PutDataLocalRelative( ResourcePutDataParams& params ) const;

		Result PutDataLocalCdn( ResourcePutDataParams& params ) const;

    protected:

        // Parameters for document version 0.0.0
		DocumentParameter<std::filesystem::path> m_relativePath = DocumentParameter<std::filesystem::path>( { 0, 0, 0 }, "RelativePath" );

		DocumentParameter<Location> m_location = DocumentParameter<Location>( { 0, 0, 0 }, "Location" );

        DocumentParameter<std::string> m_type = DocumentParameter<std::string>( { 0, 0, 0 }, "Type" );

		DocumentParameter<std::string> m_checksum = DocumentParameter<std::string>( { 0, 0, 0 }, "Checksum" );

		DocumentParameter<unsigned long> m_compressedSize = DocumentParameter<unsigned long>( { 0, 0, 0 }, "CompressedSize" );

		DocumentParameter<unsigned long> m_uncompressedSize = DocumentParameter<unsigned long>( { 0, 0, 0 }, "UncompressedSize" );

        //TODO thought exercise for document versioning, remove
		DocumentParameter<unsigned long> m_something = DocumentParameter<unsigned long>( { 0, 1, 0 }, "something" );
    };

}
#endif // ResourceInfo_H