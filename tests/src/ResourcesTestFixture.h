// Copyright © 2025 CCP ehf.

#pragma once
#ifndef CarbonResourcesTestFixture_H
#define CarbonResourcesTestFixture_H

#include <vector>

#include <gtest/gtest.h>

#include <filesystem>

#include <ResourceGroup.h>

#include <stack>

#include <memory>

struct DetailProcessStatus
{
	DetailProcessStatus( ) :
		m_progress( 0 )
	{
	}

	CarbonResources::StatusProgressType m_type;

	unsigned int m_progress;

	unsigned int m_sizeOfJob;

	std::string m_info;

};

struct ProcessStatus
{
	ProcessStatus( CarbonResources::StatusProgressType type, float sizeOfJob, unsigned int nestingLevel, const std::string& info ) :
		m_type( type ),
		m_progress( 0 ),
		m_sizeOfJob( sizeOfJob ),
		m_nestingLevel( nestingLevel ),
		m_info( info ),
		m_detailStatus( std::make_unique < DetailProcessStatus >())
	{
	}

    CarbonResources::StatusProgressType m_type;

    float m_progress;

    float m_sizeOfJob;

    unsigned int m_nestingLevel;

    std::string m_info;
    
    std::unique_ptr<DetailProcessStatus> m_detailStatus;

};

struct StatusInformation
{
	std::stack<std::unique_ptr<ProcessStatus>> processStatuses;

	float overallProgress = -1;

	bool statusStateIsValid = true;

	std::string statusStateInfo = "";
};

struct ResourcesTestFixture : public ::testing::Test
{
	void SetUp();

	void TearDown();

	std::filesystem::path GetTestFileFileAbsolutePath( const std::filesystem::path& relativePath );

	bool FileExists( const std::filesystem::path& filePath );

	bool FilesMatch( const std::filesystem::path& file1Path, const std::filesystem::path& file2Path );

	bool DirectoryIsSubset( const std::filesystem::path& dir1, const std::filesystem::path& dir2 ); // Test that all files in dir1 exist in dir2, and the contents of the files in both directories are the same.

    bool StatusIsValid();

    static void StatusUpdate( CarbonResources::StatusProgressType type, float processProgress, float overallProgress, float sizeOfJob, unsigned int nestingLevel, const std::string& info );

    static inline thread_local StatusInformation s_statusInformation;

};

#endif // CarbonResourcesTestFixture_H