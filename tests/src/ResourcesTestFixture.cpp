// Copyright © 2025 CCP ehf.

#include "ResourcesTestFixture.h"

#include <process.hpp>

#include <iostream>

#include <fstream>

#include <sstream>

#include <ResourceTools.h>

void ResourcesTestFixture::SetUp()
{

}

void ResourcesTestFixture::TearDown()
{
}

bool ResourcesTestFixture::FileExists( const std::filesystem::path& filePath )
{
	return std::filesystem::exists( filePath );
}

bool ResourcesTestFixture::FilesMatch( const std::filesystem::path& file1Path, const std::filesystem::path& file2Path )
{
	// Open files generate data checksums and compare

	// File 1
	std::ifstream inputStream1;

	inputStream1.open( file1Path, std::ios::in | std::ios::binary );

	if( !inputStream1 )
	{
		return false;
	}

	std::stringstream ss1;

	ss1 << inputStream1.rdbuf();

	std::string file1Data = ss1.str();

	inputStream1.close();

	std::string file1Checksum = "";

	bool result1 = ResourceTools::GenerateMd5Checksum( file1Data, file1Checksum );


	// File 2
	std::ifstream inputStream2;

	inputStream2.open( file2Path, std::ios::in | std::ios::binary );

	if( !inputStream2 )
	{
		return false;
	}

	std::stringstream ss2;

	ss2 << inputStream2.rdbuf();

	std::string file2Data = ss2.str();

	inputStream2.close();

	std::string file2Checksum = "";

	bool result2 = ResourceTools::GenerateMd5Checksum( file2Data, file2Checksum );

	return file1Checksum == file2Checksum;
}

bool ResourcesTestFixture::DirectoryIsSubset( const std::filesystem::path& dir1, const std::filesystem::path& dir2 )
{
	for( auto entry : std::filesystem::recursive_directory_iterator( dir1 ) )
	{
		if( entry.is_directory() )
		{
			continue;
		}
		if( !FilesMatch( entry.path(), dir2 / relative( entry.path(), dir1 ) ) )
		{
			return false;
		}
	}
	return true;
}

bool ResourcesTestFixture::StatusIsValid()
{
	return s_statusInformation.statusStateIsValid;
}

bool FloatsAreEqual(const float v1, const float v2)
{
	return abs( v1 - v2 ) <= 1e-4f;
}

void ResourcesTestFixture::StatusUpdate( CarbonResources::StatusProgressType type, float processProgress, float overallProgress, float sizeOfJob, unsigned int nestingLevel, const std::string& info )
{
	if( !s_statusInformation.statusStateIsValid )
    {
        // status state already reached an invalid state
        // stop checks at first failure
		return;
    }

    // Check a type was not a warning
    if (type == CarbonResources::StatusProgressType::WARNING)
    {
        // Warnings are ignored
		return;
    }

    // Check start of an overall process test
	if( s_statusInformation.overallProgress < 0 )
    {
        // Ensure that the first overall progress update is zero
        if (overallProgress != 0)
        {
			// The progress should only increase and never decrease
			s_statusInformation.statusStateIsValid = false;
			s_statusInformation.statusStateInfo = "The overall progress started above the expected zero";
			return;
        }

    }
    else
    {
        // Ensure that overall progress never decreases
        // It can remain the same
		if( overallProgress < s_statusInformation.overallProgress )
        {
			// Overall progress should never decrease
			s_statusInformation.statusStateIsValid = false;
			s_statusInformation.statusStateInfo = "Overall progress decreased. Could be caused by out of order updates, or could there be a parent update missing in between nested jobs of the same level?";
			return;
        }
    }

    if( s_statusInformation.overallProgress > 100 )
    {
		// Overall progress should never exceed 100
		s_statusInformation.statusStateIsValid = false;
		s_statusInformation.statusStateInfo = "Overall progress cannot exceed 100";
		return;
    }

    // Store current state of overall progress
	s_statusInformation.overallProgress = overallProgress;

    // Test integrity of status updates
	if( s_statusInformation.processStatuses.empty() )
    {
		// First process encountered
		s_statusInformation.processStatuses.push( std::make_unique<ProcessStatus>( type, sizeOfJob, nestingLevel, info ) );

        return;
    }

	ProcessStatus* lastStatus = s_statusInformation.processStatuses.top().get();

    if (type == CarbonResources::StatusProgressType::UNBOUNDED)
    {
        if (lastStatus->m_nestingLevel == nestingLevel)
        {
            // Continue, progress is unbounded
			return;
        }
    }

    if (type == CarbonResources::StatusProgressType::START)
    {
        if (processProgress != 0)
        {
            // First update must have initial progress of zero
			s_statusInformation.statusStateIsValid = false;
			s_statusInformation.statusStateInfo = "New process started with a non zero progress initial call.";
			return;
        }

        if( sizeOfJob != 0 )
		{
			// First update only makes sense to have size of job be 0
			s_statusInformation.statusStateIsValid = false;
			s_statusInformation.statusStateInfo = "New processes should use a job size of 0, anything higher doesn't make sense for the status system.";
			return;
		}

        if (lastStatus->m_nestingLevel == nestingLevel)
        {
			// New process must have a higher nesting level than previous process
			s_statusInformation.statusStateIsValid = false;
			s_statusInformation.statusStateInfo = "New process must have a higher nesting level than the previous process. Perhaps percentage update is incorrect?";
			return;
        }

        // Indicates that a new job has been started or a process detail status has begun
		s_statusInformation.processStatuses.push( std::make_unique<ProcessStatus>( type, sizeOfJob, nestingLevel, info ) );

        return;
    }
    else
    {
        // See if this is an update to the current process or a nested process starting

        // Ensure that new progress is valid
        if (processProgress < lastStatus->m_progress)
        {
            // The progress should only increase and never decrease
			s_statusInformation.statusStateIsValid = false;
			s_statusInformation.statusStateInfo = "A processes progress appeared to decrease status calls.";
			return;
        }

        // Ensure the nesting level is expected
        if (lastStatus->m_nestingLevel > nestingLevel)
        {
            // The nesting level is expected to not have increased
			s_statusInformation.statusStateIsValid = false;
			s_statusInformation.statusStateInfo = "A nested process progress was updated before an initial call with new nest level was started \
                                                    or a child process fully completed";
			return;
        }

        // Update progress
		lastStatus->m_progress = processProgress;

        // Check for process finishing but with incorrect end type
		if(( processProgress == 100 )
			&& (type != CarbonResources::StatusProgressType::END))
		{
			s_statusInformation.statusStateIsValid = false;
			s_statusInformation.statusStateInfo = "Progress of 100 encountered with status type that is not StatusProgressType::END.";
			return;
		}

        // Check end of process
        if (type == CarbonResources::StatusProgressType::END)
        {
			// Remove the process from stack
			s_statusInformation.processStatuses.pop();

            // Reset overall progress state if this was the last status
			if( s_statusInformation.processStatuses.empty() )
            {
                // Check that overall progress was approximately 100
				if( !FloatsAreEqual( s_statusInformation.overallProgress, 100 ) )
                {
					s_statusInformation.statusStateIsValid = false;
					s_statusInformation.statusStateInfo = "At the end of the last process, overall progress was not approximately 100";
					return;
                }

				s_statusInformation.overallProgress = -1;
            }
   
        }
    }
}

std::filesystem::path ResourcesTestFixture::GetTestFileFileAbsolutePath( const std::filesystem::path& relativePath )
{
	std::filesystem::path basePath( TEST_DATA_BASE_PATH );

	return basePath / relativePath;
}