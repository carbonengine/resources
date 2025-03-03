/* 
	*************************************************************************

	Resource.h

	Author:    James Hawk
	Created:   Feb. 2025
	Project:   Resources

	Description:   

	  

	(c) CCP 2025

	*************************************************************************
*/

#pragma once
#ifndef Resource_H
#define Resource_H

#include "Exports.h"
#include "Enums.h"
#include <memory>
#include <string>
#include <optional>
#include <vector>

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
        DocumentParameter(Version version, const std::string& tag):
			m_version(version),
			m_tag(tag)
        {
        }

        void operator=(T value)
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

        bool IsParameterExpectedInDocumentVersion(Version documentVersion) const
        {
			return documentVersion >= m_version;
        }

        std::string GetTag( ) const
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

        void PushBack(T attribute)
        {
			m_collection.push_back( attribute );
        }

        void Remove( typename std::vector<T>::const_iterator attributeIterator )
        {
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

    class API ResourceParams
    {
	public:
		ResourceParams();

        virtual Result ImportFromYaml( YAML::Node& resource, const Version& documentVersion ); //TODO Get out of public api

        // Parameters for document version 0.0.0
        DocumentParameter<std::string> relativePath = DocumentParameter<std::string>( { 0, 0, 0 }, "RelativePath" );

        DocumentParameter<std::string> location = DocumentParameter<std::string>( { 0, 0, 0 }, "Location" );

        DocumentParameter<std::string> checksum = DocumentParameter<std::string>( { 0, 0, 0 }, "Checksum" );

        DocumentParameter<unsigned long> compressedSize = DocumentParameter<unsigned long>( { 0, 0, 0 }, "CompressedSize" );

        DocumentParameter<unsigned long> uncompressedSize = DocumentParameter<unsigned long>( { 0, 0, 0 }, "UncompressedSize" );

        // TODO remove, just for thought exercise of how a parameter could be added which causes a binary missmatch and handling that
        // See macro in cpp file
        DocumentParameter<unsigned long> something = DocumentParameter<unsigned long>( { 0, 1, 0 }, "something" );

    };

    struct API ResourceGetDataParams
	{
		std::string basePath = "";

        std::string* data;

	};

    class API Resource
    {
    protected:
        class ResourceImpl;

        Resource( ResourceImpl* impl );

        ResourceImpl* m_impl;

    public:
		Resource( const ResourceParams& params );

	    virtual ~Resource();

        virtual Result ExportToYaml( YAML::Emitter& out, const Version& documentVersion );

        bool operator==( const Resource& other ) const
		{
			return GetRelativePath().GetValue() == other.GetRelativePath().GetValue();
		}

        DocumentParameter<std::string> GetRelativePath() const;

        DocumentParameter<std::string> GetLocation() const;

        DocumentParameter<std::string> GetChecksum() const;

        DocumentParameter<unsigned long> GetUncompressedSize() const;

        DocumentParameter<unsigned long> GetCompressedSize() const;

        DocumentParameter<unsigned long> GetSomething() const;

		Result GetData( const ResourceGetDataParams& params ) const;

    };

}

#endif // Resource_H