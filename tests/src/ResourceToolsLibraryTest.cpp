#include <ResourceGroup.h>
#include <BundleResourceGroup.h>
#include <PatchResourceGroup.h>
#include <ResourceTools.h>
#include <filesystem>

#include <gtest/gtest.h>

TEST( ResourceToolsTest, Md5ChecksumGeneration )
{
	std::string input = "Dummy";
	std::string output = "";

    EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( input, output ) );

    EXPECT_EQ( output, "bcf036b6f33e182d4705f4f5b1af13ac" );
}

TEST( ResourceToolsTest, FowlerNollVoChecksumGeneration )
{
	std::string input = "res:/intromovie.txt";
	std::string output = "";

	EXPECT_TRUE( ResourceTools::GenerateFowlerNollVoChecksum( input, output ) );

	EXPECT_EQ( output, "a9d1721dd5cc6d54" );
}

TEST( ResourceToolsTest, DownloadFile )
{
	const char* FOLDER_NAME = "a9";
	const char* FILE_NAME = "a9d1721dd5cc6d54_e6bbb2df307e5a9527159a4c971034b5";

	const char* testDataPathStr = std::getenv( "TEST_DATA_PATH" );
	ASSERT_TRUE( testDataPathStr );
	std::filesystem::path testDataPath(testDataPathStr);
	ResourceTools::Initialize();

	std::filesystem::path sourcePath = testDataPath / "resourcesLocal" / FOLDER_NAME / FILE_NAME;
	std::string sourcePathString(sourcePath.string());
	std::string url = "file://" + sourcePathString;
	std::filesystem::path outputPath = std::filesystem::temp_directory_path() / FOLDER_NAME / FILE_NAME;
	std::string outputPathString = outputPath.string();

	if( std::filesystem::exists( outputPath ) )
	{
		// Nuke any potentially pre-existing file.
		std::filesystem::remove( outputPath );
	}
	EXPECT_FALSE( std::filesystem::exists( outputPath ) );
	EXPECT_TRUE( ResourceTools::DownloadFile( url, outputPathString ) );
	EXPECT_TRUE( std::filesystem::exists( outputPath ) );

	// Check if download succeeds.
	std::string downloadedData;
	bool success = ResourceTools::GetLocalFileData( outputPathString, downloadedData );
	EXPECT_TRUE( success );

	// Verify downloaded file contents.
	std::string checksum;
	ResourceTools::GenerateMd5Checksum( downloadedData, checksum );
	EXPECT_STREQ( checksum.c_str(), "6ccf6b7e2e263646f5a78e77b9ba3168" );
	ResourceTools::ShutDown();
}

TEST( ResourceToolsTest, GZipCompressData )
{
	std::string inputDataToCompress = "SomeData";

    std::string outputData = "";

	EXPECT_TRUE( ResourceTools::GZipCompressData( inputDataToCompress, outputData ) );

	// TODO check data using real data and data checksum
}

TEST( ResourceToolsTest, GZipUncompressData )
{
	std::string inputDataToUncompress = "SomeData";

	std::string outputData = "";

	EXPECT_TRUE( ResourceTools::GZipUncompressData( inputDataToUncompress, outputData ) );

	// TODO check data using real data and data checksum
}