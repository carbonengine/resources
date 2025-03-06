#include "CarbonResourcesTestFixture.h"

#include <process.hpp>

#include <iostream>

#include <fstream>

#include <sstream>

#include <ResourceTools.h>

#include "Paths.h"

void CarbonResourcesTestFixture::SetUp()
{

}

void CarbonResourcesTestFixture::TearDown()
{
	
}

bool CarbonResourcesTestFixture::BundleIsValid()
{
    // TODO - This will check that the bundleResourceGroup file is valid
    // It will also check all the files in the list
    // And check all their checksums
    // Should it unbundle and check that too?
	return false;
}

bool CarbonResourcesTestFixture::PatchIsValid()
{
    // TODO will do similar to above,
    // Probably there should be a function for resourceGroup is valid too and these utilise that.
	return false;
}

bool CarbonResourcesTestFixture::FilesMatch( const std::filesystem::path& file1Path, const std::filesystem::path& file2Path )
{
    // Open files generate data checksums and compare

    // File 1
	std::ifstream inputStream1;

	inputStream1.open( file1Path, std::ios::in | std::ios::binary );

    if (!inputStream1)
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

    if (!inputStream2)
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

std::filesystem::path CarbonResourcesTestFixture::GetTestFileFileAbsolutePath( const std::filesystem::path& relativePath )
{
    std::filesystem::path basePath( TEST_DATA_BASE_PATH );

    return basePath / relativePath;
}