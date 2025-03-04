/* 
	*************************************************************************

	ResourceImpl.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/
#pragma once
#ifndef ResourceImpl_H
#define ResourceImpl_H

#include "Resource.h"
#include <string>
#include <sstream>

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

	    bool HasValue()
	    {
		    return m_value.has_value();
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

    struct RelativePath
    {
        RelativePath(std::string pathStr)
        {
			FromString( pathStr );  //TODO this can fail
        }

        
        bool FromString(const std::string& pathStr)
        {
			
			size_t separatorPosition = pathStr.find( ":/" );

            if (separatorPosition == std::string::npos)
            {
				return false;
            }

            prefix = pathStr.substr( 0, separatorPosition );

            filename = pathStr.substr( separatorPosition + 2 );
            
			return true;
        }
        

	    bool operator==( const RelativePath& other ) const
	    {
		    return ( prefix == other.prefix ) && ( filename == other.filename );
	    }

        std::string ToString()
        {
			std::stringstream ss;

            ss << prefix << ":/" << filename;

            return ss.str();
        }

	    std::string prefix = "";
	    std::string filename = "";
    };

    class ResourceImpl
    {
    public:
	    ResourceImpl( const ResourceParams& params );

	    ~ResourceImpl();

	    DocumentParameter<RelativePath> GetRelativePath() const;

	    void SetRelativePath( const RelativePath& relativePath );

	    DocumentParameter<std::string> GetLocation() const;

	    DocumentParameter<std::string> GetChecksum() const;

	    DocumentParameter<unsigned long> GetUncompressedSize() const;

	    DocumentParameter<unsigned long> GetCompressedSize() const;

	    DocumentParameter<unsigned long> GetSomething() const;

	    Result GetData( ResourceGetDataParams& params ) const;

        virtual Result ImportFromYaml( YAML::Node& resource, const Version& documentVersion ); 

        virtual Result ExportToYaml( YAML::Emitter& out, const Version& documentVersion );

    private:
	    Result GetDevelopmentLocalData( ResourceGetDataParams& params ) const;

	    Result GetProductionLocalData( ResourceGetDataParams& params ) const;

	    Result GetProductionRemoteData( ResourceGetDataParams& params ) const;

    private:

        // Parameters for document version 0.0.0
		DocumentParameter<RelativePath> m_relativePath = DocumentParameter<RelativePath>( { 0, 0, 0 }, "RelativePath" );

		DocumentParameter<std::string> m_location = DocumentParameter<std::string>( { 0, 0, 0 }, "Location" );

		DocumentParameter<std::string> m_checksum = DocumentParameter<std::string>( { 0, 0, 0 }, "Checksum" );

		DocumentParameter<unsigned long> m_compressedSize = DocumentParameter<unsigned long>( { 0, 0, 0 }, "CompressedSize" );

		DocumentParameter<unsigned long> m_uncompressedSize = DocumentParameter<unsigned long>( { 0, 0, 0 }, "UncompressedSize" );

		// TODO remove, just for thought exercise of how a parameter could be added which causes a binary missmatch and handling that
		// See macro in cpp file
		DocumentParameter<unsigned long> m_something = DocumentParameter<unsigned long>( { 0, 1, 0 }, "something" );
    };

}
#endif // ResourceImpl_H