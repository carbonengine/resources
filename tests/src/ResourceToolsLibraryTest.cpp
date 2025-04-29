#include <ResourceGroup.h>
#include <BundleResourceGroup.h>
#include <PatchResourceGroup.h>
#include <ResourceTools.h>
#include <BundleStreamOut.h>
#include <filesystem>

#include <gtest/gtest.h>

#include "CarbonResourcesTestFixture.h"
#include "FileDataStreamIn.h"
#include "GzipCompressionStream.h"
#include "GzipDecompressionStream.h"
#include "Md5ChecksumStream.h"
#include "Patching.h"

struct ResourceToolsTest : public CarbonResourcesTestFixture
{
};

TEST_F( ResourceToolsTest, Md5ChecksumGeneration )
{
	std::string input = "Dummy";
	std::string output = "";

    EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( input, output ) );

    EXPECT_EQ( output, "bcf036b6f33e182d4705f4f5b1af13ac" );
}

TEST_F( ResourceToolsTest, FowlerNollVoChecksumGeneration )
{
	std::string input = "res:/intromovie.txt";
	std::string output = "";

	EXPECT_TRUE( ResourceTools::GenerateFowlerNollVoChecksum( input, output ) );

	EXPECT_EQ( output, "a9d1721dd5cc6d54" );
}

TEST_F( ResourceToolsTest, DownloadFile )
{
	const char* FOLDER_NAME = "a9";
	const char* FILE_NAME = "a9d1721dd5cc6d54_e6bbb2df307e5a9527159a4c971034b5";

	const char* testDataPathStr = std::getenv( "TEST_DATA_PATH" );
	ASSERT_TRUE( testDataPathStr );
	ResourceTools::Downloader downloader;
	std::filesystem::path testDataPath(testDataPathStr);

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
	EXPECT_TRUE( downloader.DownloadFile( url, outputPathString ) );
	EXPECT_TRUE( std::filesystem::exists( outputPath ) );

	// Check if download succeeds.
	std::string downloadedData;
	bool success = ResourceTools::GetLocalFileData( outputPathString, downloadedData );
	EXPECT_TRUE( success );

	// Verify downloaded file contents.
	// Note that in this example we are not downloading from an actual web server
	// In production, the web server would return a reply with `Content-Encoding`: `gzip`
	// which would cause curl to automatically unzip the thing for us, but in order to
	// keep the test small and isolated we do not exercise this functionality.
	std::string checksum;
	ResourceTools::GenerateMd5Checksum( downloadedData, checksum );
	EXPECT_STREQ( checksum.c_str(), "6ccf6b7e2e263646f5a78e77b9ba3168" );
}

TEST_F( ResourceToolsTest, GZipCompressString )
{
	std::string inputDataToCompress = "SomeData";
	std::string outputData = "";
	EXPECT_TRUE( ResourceTools::GZipCompressData( inputDataToCompress, outputData ) );
	EXPECT_EQ( outputData.substr( 0, 2 ), "\x1F\x8B"); // Start of GZIP header.
}

TEST_F( ResourceToolsTest, GZipCompressData )
{
	const char* FOLDER_NAME = "a9";
	const char* FILE_NAME = "a9d1721dd5cc6d54_e6bbb2df307e5a9527159a4c971034b5";
	const int GZIP_HEADER_BYTES = 10; // The size of a standard gzip header with no "optional" fields.
	const int FILENAME_BYTES = strlen( FILE_NAME ) + 1; // Number of bytes that the "optional" filename field takes up in the header. 49 characters and one '\0' byte

	const char* testDataPathStr = std::getenv( "TEST_DATA_PATH" );
	ASSERT_TRUE( testDataPathStr );
	std::filesystem::path testDataPath( testDataPathStr );
	std::filesystem::path zippedSourcePath = testDataPath / "resourcesLocal" / FOLDER_NAME / FILE_NAME;
	std::filesystem::path unzippedSourcePath = testDataPath / "resourcesOnBranch" / "introMovie.txt";

	std::string zippedFileData;
	ASSERT_TRUE( ResourceTools::GetLocalFileData( zippedSourcePath, zippedFileData ) );

	std::string unzippedFileData;
	ASSERT_TRUE( ResourceTools::GetLocalFileData( unzippedSourcePath, unzippedFileData ) );

	std::string compressed;
	EXPECT_TRUE( ResourceTools::GZipCompressData( unzippedFileData, compressed ) );

	// Check that the compressed file matches the data in the file we have on disk
	// EXCEPT for the header.
	// https://docs.fileformat.com/compression/gz/
	std::string compressedNoHeader = compressed.substr( GZIP_HEADER_BYTES );
	std::string zippedFileDataNoHeader = zippedFileData.substr( GZIP_HEADER_BYTES + FILENAME_BYTES );
	EXPECT_EQ( compressedNoHeader, zippedFileDataNoHeader );
}

TEST_F( ResourceToolsTest, GZipUncompressString )
{
	std::string inputDataToUncompress( "\x1F\x8B\b\0\0\0\0\0\x2\n\v\xCE\xCFMuI,I\x4\0\xB8pH\n\b\0\0\0", 28 );
	std::string outputData = "";
	EXPECT_TRUE( ResourceTools::GZipUncompressData( inputDataToUncompress, outputData ) );
	EXPECT_EQ( outputData, "SomeData" );
}

TEST_F( ResourceToolsTest, GZipUncompressData )
{
	const char* FOLDER_NAME = "a9";
	const char* FILE_NAME = "a9d1721dd5cc6d54_e6bbb2df307e5a9527159a4c971034b5";
	const char* testDataPathStr = std::getenv( "TEST_DATA_PATH" );
	ASSERT_TRUE( testDataPathStr );

	std::filesystem::path testDataPath( testDataPathStr );
	std::filesystem::path zippedSourcePath = testDataPath / "resourcesLocal" / FOLDER_NAME / FILE_NAME;
	std::filesystem::path unzippedSourcePath = testDataPath / "resourcesOnBranch" / "introMovie.txt";

	// Load gzipped file.
	std::string zippedFileData;
	ASSERT_TRUE( ResourceTools::GetLocalFileData( zippedSourcePath, zippedFileData ) );

	// Decompress the file.
	std::string decompressed;
	EXPECT_TRUE( ResourceTools::GZipUncompressData( zippedFileData, decompressed ) );

	// Make sure decompressed file matches expected contents.
	std::string checksum;
	ResourceTools::GenerateMd5Checksum( decompressed, checksum );
	EXPECT_EQ( checksum, "e6bbb2df307e5a9527159a4c971034b5" );
}

TEST_F( ResourceToolsTest, ResourceChunking )
{
	uintmax_t chunkSize = 1000;

	ResourceTools::BundleStreamOut bundleStream(chunkSize);

    // Add test resource1 data
	std::string resource1Data;

    std::filesystem::path resource1Path = GetTestFileFileAbsolutePath( "Bundle/TestResources/One.png" );

    EXPECT_TRUE(ResourceTools::GetLocalFileData( resource1Path, resource1Data ));

    EXPECT_TRUE(bundleStream << resource1Data);

    std::string resource1Checksum;

    EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( resource1Data, resource1Checksum ));

    // Add test resource2 data
	std::string resource2Data;

	std::filesystem::path resource2Path = GetTestFileFileAbsolutePath( "Bundle/TestResources/Two.png" );

	EXPECT_TRUE( ResourceTools::GetLocalFileData( resource2Path, resource2Data ) );

	EXPECT_TRUE( bundleStream << resource2Data );

    std::string resource2Checksum;

    EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( resource2Data, resource2Checksum ) );

    // Add test resource3 data
	std::string resource3Data;

	std::filesystem::path resource3Path = GetTestFileFileAbsolutePath( "Bundle/TestResources/Three.png" );

	EXPECT_TRUE( ResourceTools::GetLocalFileData( resource3Path, resource3Data ) );

	EXPECT_TRUE( bundleStream << resource3Data );

    std::string resource3Checksum;

	EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( resource3Data, resource3Checksum ) );

    // Get chunks
	int numberOfChunks = 0;

	std::string chunkData;

	ResourceTools::GetChunk chunk;

    chunk.data = &chunkData;

    chunk.clearCache = false;

    while (bundleStream >> chunk)
    {
        // Create Filename
		std::stringstream ss;

        ss << "Chunks/Chunk";

        ss << numberOfChunks;

        ss << ".chunk";

        std::string chunkPath = ss.str();

        // Save chunks
		EXPECT_TRUE( ResourceTools::SaveFile( chunkPath, chunkData ));

        numberOfChunks++;
    }

    // Clear cache for last chunk
	chunk.clearCache = true;

	EXPECT_TRUE( bundleStream >> chunk );

    // Create Filename
	std::stringstream ss;

	ss << "Chunks/Chunk";

	ss << numberOfChunks;

	ss << ".chunk";

	std::string chunkPath = ss.str();

	// Save chunk
	EXPECT_TRUE( ResourceTools::SaveFile( chunkPath, chunkData ) );


    // TODO reimplement this

    /*

	// Reconsitute the files
	ResourceTools::ChunkStream chunkStreamReconstitute( chunkSize );

    for (int i = 0; i < numberOfChunks + 1; i++)
    {
		// Create Filename
		std::stringstream ss;

		ss << "Chunks/Chunk";

		ss << i;

		ss << ".chunk";

		std::string chunkPath = ss.str();

        // Get chunks
		std::string chunkData;

		EXPECT_TRUE( ResourceTools::GetLocalFileData(chunkPath,chunkData));

        EXPECT_TRUE( chunkStreamReconstitute << chunkData );

    }

    // Reconstitute the files and check they match original

    // Resource 1
	std::string reconstitutedResource1Data;

	ResourceTools::GetFile resource1File;

    resource1File.data = &reconstitutedResource1Data;

    resource1File.fileSize = resource1Data.size();

    EXPECT_TRUE( chunkStreamReconstitute >> resource1File );

	std::string reconstitutedResource1Checksum;

	EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( reconstitutedResource1Data, reconstitutedResource1Checksum ) );

    EXPECT_EQ( resource1Checksum, reconstitutedResource1Checksum );

    EXPECT_TRUE( ResourceTools::SaveFile( "Chunks/One.png", reconstitutedResource1Data ) );


    // Resource 2
	std::string reconstitutedResource2Data;

	ResourceTools::GetFile resource2File;

	resource2File.data = &reconstitutedResource2Data;

	resource2File.fileSize = resource2Data.size();

	EXPECT_TRUE( chunkStreamReconstitute >> resource2File );

	std::string reconstitutedResource2Checksum;

	EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( reconstitutedResource2Data, reconstitutedResource2Checksum ) );

	EXPECT_EQ( resource2Checksum, reconstitutedResource2Checksum );

    EXPECT_TRUE( ResourceTools::SaveFile( "Chunks/Two.png", reconstitutedResource2Data ) );


    // Resource 3
	std::string reconstitutedResource3Data;

	ResourceTools::GetFile resource3File;

	resource3File.data = &reconstitutedResource3Data;

	resource3File.fileSize = resource3Data.size();

	EXPECT_TRUE( chunkStreamReconstitute >> resource3File );

	std::string reconstitutedResource3Checksum;

	EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( reconstitutedResource3Data, reconstitutedResource3Checksum ) );

	EXPECT_EQ( resource3Checksum, reconstitutedResource3Checksum );

    EXPECT_TRUE( ResourceTools::SaveFile( "Chunks/Three.png", reconstitutedResource3Data ) );

    */

}


TEST_F( ResourceToolsTest, GZipUncompressTestFile )
{

    std::filesystem::path resourcePath = GetTestFileFileAbsolutePath( "CompressedFiles/ab5cde4fbbf82fb6_6a9d6d4c6015616877b77865209c5064" );

    std::string resourceData;

    EXPECT_TRUE( ResourceTools::GetLocalFileData( resourcePath, resourceData ) );

    std::string uncompressedData;

    EXPECT_TRUE( ResourceTools::GZipUncompressData( resourceData, uncompressedData ) );

    // Calculate the checksum to check it
	std::string checksum;

    EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( uncompressedData, checksum ) );

    // Save the file
	std::filesystem::path resourcePathDest = "GZipOut/ab/ab5cde4fbbf82fb6_6a9d6d4c6015616877b77865209c5064";

	EXPECT_TRUE( ResourceTools::SaveFile( resourcePathDest, uncompressedData ) );
}

TEST_F( ResourceToolsTest, CreateApplyPatch )
{
	std::string before = "acbd";
	std::string after = "abcd";
	std::string patch = "";

	// Create a patch containing the difference between before and after.
	ASSERT_TRUE( ResourceTools::CreatePatch( before, after, patch ) );

	std::string patched = "";
	// Apply the patch to before
	ASSERT_TRUE(  ResourceTools::ApplyPatch( before, patch, patched ) );

	// After patching before, it should be exactly the same as after.
	ASSERT_EQ( patched, after );

}

TEST_F( ResourceToolsTest, CreateApplyPatchFile )
{
	const char* testDataPathStr = std::getenv( "TEST_DATA_PATH" );
	ASSERT_TRUE( testDataPathStr );
	std::filesystem::path testDataPath(testDataPathStr);

	std::filesystem::path before_src = testDataPath / "Patch" / "PreviousBuildResources" / "introMovie.txt";
	std::filesystem::path after_src = testDataPath / "Patch" / "NextBuildResources" / "introMovie.txt";

	std::filesystem::path patch = std::filesystem::temp_directory_path() / "CarbonResources" / "introMovie.patch";
	std::filesystem::path before = std::filesystem::temp_directory_path() / "CarbonResources" /  "introMovie.txt";

	std::filesystem::remove( before );
	std::filesystem::remove( patch );
	std::filesystem::copy_file( before_src, before );

	// Create a patch containing the difference between before and after.
	ASSERT_TRUE( ResourceTools::CreatePatchFile( before_src, after_src, patch ) );

	std::string patched = "";
	// Apply the patch to before
	ASSERT_TRUE(  ResourceTools::ApplyPatchFile( before, patch ) );

	std::string beforeChecksum;
	std::string beforeData;
	ResourceTools::GetLocalFileData( before, beforeData );
	EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( beforeData, beforeChecksum ) );

	std::string afterChecksum;
	std::string afterData;
	ResourceTools::GetLocalFileData( after_src, afterData );
	EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( afterData, afterChecksum ) );

	// After patching before, it should be exactly the same as after.
	ASSERT_EQ( beforeChecksum, afterChecksum );
}

TEST_F( ResourceToolsTest, ApplyPatchFileChunked )
{
	const char* testDataPathStr = std::getenv( "TEST_DATA_PATH" );
	ASSERT_TRUE( testDataPathStr );
	std::filesystem::path testDataPath(testDataPathStr);

	std::filesystem::path before_src = testDataPath / "Patch" / "PreviousBuildResources" / "introMovie.txt";
	std::filesystem::path after_src = testDataPath / "Patch" / "NextBuildResources" / "introMovie.txt";

	std::filesystem::path patch = std::filesystem::temp_directory_path() / "CarbonResources" / "introMovie.patch";
	std::filesystem::path before = std::filesystem::temp_directory_path() / "CarbonResources" /  "introMovie.txt";

	std::filesystem::remove( before );
	std::filesystem::remove( patch );
	std::filesystem::copy_file( before_src, before );

	// Create a patch containing the difference between before and after.
	ASSERT_TRUE( ResourceTools::CreatePatchFile( before_src, after_src, patch ) );

	uintmax_t chunkSize = 128;

	ResourceTools::BundleStreamOut chunkStream(chunkSize);
	std::string patchData;

	EXPECT_TRUE(ResourceTools::GetLocalFileData( patch, patchData ));

	EXPECT_TRUE(chunkStream << patchData);
	// Apply the patch to before
	ASSERT_TRUE(  ResourceTools::ApplyPatchFileChunked( before, chunkStream ) );


	std::string beforeChecksum;
	std::string beforeData;
	ResourceTools::GetLocalFileData( before, beforeData );
	EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( beforeData, beforeChecksum ) );

	std::string afterChecksum;
	std::string afterData;
	ResourceTools::GetLocalFileData( after_src, afterData );
	EXPECT_TRUE( ResourceTools::GenerateMd5Checksum( afterData, afterChecksum ) );

	// After patching before, it should be exactly the same as after.
	ASSERT_EQ( beforeChecksum, afterChecksum );
}

TEST_F( ResourceToolsTest, RollingChecksum )
{
	std::string data("01234asdf567asdf89");
	ResourceTools::RollingChecksum first = ResourceTools::GenerateRollingAdlerChecksum(data, 0, 5);
	ResourceTools::RollingChecksum second = ResourceTools::GenerateRollingAdlerChecksum(data, 5, 9);
	ResourceTools::RollingChecksum third = ResourceTools::GenerateRollingAdlerChecksum(data, 9, 12);
	ResourceTools::RollingChecksum fourth = ResourceTools::GenerateRollingAdlerChecksum(data, 12, 16);
	ResourceTools::RollingChecksum fifth = ResourceTools::GenerateRollingAdlerChecksum(data, 16, 18);

	ASSERT_NE( first.checksum, second.checksum );
	ASSERT_NE( first.checksum, third.checksum );
	ASSERT_NE( first.checksum, fifth.checksum );
	ASSERT_NE( second.checksum, third.checksum );
	ASSERT_EQ( second.checksum, fourth.checksum );
	ASSERT_NE( second.checksum, fifth.checksum );
	ASSERT_NE( third.checksum, fourth.checksum );
	ASSERT_NE( third.checksum, fifth.checksum );
	ASSERT_NE( fourth.checksum, fifth.checksum );

	ResourceTools::RollingChecksum previous = ResourceTools::GenerateRollingAdlerChecksum(data, 0, 4);
	for( int i = 1; i < 12; ++i )
	{
		ResourceTools::RollingChecksum fromScratch = ResourceTools::GenerateRollingAdlerChecksum(data, i, i + 4);
		ResourceTools::RollingChecksum incremental = ResourceTools::GenerateRollingAdlerChecksum(data, i, i + 4, previous);
		ASSERT_EQ( fromScratch.alpha, incremental.alpha );
		ASSERT_EQ( fromScratch.beta, incremental.beta );
		ASSERT_EQ( fromScratch.checksum, incremental.checksum );
		previous = fromScratch;
	}
}

TEST_F( ResourceToolsTest, FindMatchingChunksEverythingMatches )
{
	std::string source("0123456789");
	std::string destination("0123456789");
	std::list<ResourceTools::ChunkMatch> result = ResourceTools::FindMatchingChunks(source, destination);
	ASSERT_EQ( 1, result.size() );
	auto match = result.front();
	ASSERT_EQ( match.sourceOffset, 0 );
	ASSERT_EQ( match.destinationOffset, 0 );
	ASSERT_EQ( match.length, 10 );
}

TEST_F( ResourceToolsTest, FindMatchingChunksShorterDestination )
{
	std::string source("0123456789");
	std::string destination("01234");
	std::list<ResourceTools::ChunkMatch> result = ResourceTools::FindMatchingChunks(source, destination);
	ASSERT_EQ( 1, result.size() );
	auto match = result.front();
	ASSERT_EQ( match.sourceOffset, 0 );
	ASSERT_EQ( match.destinationOffset, 0 );
	ASSERT_EQ( match.length, 5 );
}

TEST_F( ResourceToolsTest, FindMatchingChunksShorterSource )
{
	std::string source("01234");
	std::string destination("0123456789");
	std::list<ResourceTools::ChunkMatch> result = ResourceTools::FindMatchingChunks(source, destination);
	ASSERT_EQ( 1, result.size() );
	auto match = result.front();
	ASSERT_EQ( match.sourceOffset, 0 );
	ASSERT_EQ( match.destinationOffset, 0 );
	ASSERT_EQ( match.length, 5 );
}

TEST_F( ResourceToolsTest, FindMatchingChunksInString )
{
	std::string source("abc3456ij");
	std::string destination("0123456789");
	std::list<ResourceTools::ChunkMatch> result = ResourceTools::FindMatchingChunks(source, destination);
	ASSERT_EQ( 1, result.size() );
	auto match = result.front();
	ASSERT_EQ( match.sourceOffset, 3 );
	ASSERT_EQ( match.destinationOffset, 3 );
	ASSERT_EQ( match.length, 4 );
}

TEST_F( ResourceToolsTest, FindMatchingChunkInFile )
{
	const char* testDataPathStr = std::getenv( "TEST_DATA_PATH" );
	ASSERT_TRUE( testDataPathStr );
	std::filesystem::path testDataPath(testDataPathStr);
    std::filesystem::path introMovieFilePath = testDataPath / "ResourcesOnBranch" / "introMovie.txt";
	std::string data;
	ResourceTools::GetLocalFileData( introMovieFilePath, data );

	size_t offset;

	std::string notInFile = "Once upon a time, in a galaxy far, far away...";
	ASSERT_FALSE(ResourceTools::FindMatchingChunk( notInFile, introMovieFilePath, offset ) );

	std::string startOfFile = "TIME";
	ASSERT_TRUE(ResourceTools::FindMatchingChunk( startOfFile, introMovieFilePath, offset ) );
	ASSERT_EQ( offset, 0 );

	std::string early = "introseq.blue";
	ASSERT_TRUE(ResourceTools::FindMatchingChunk( early, introMovieFilePath, offset ) );
	ASSERT_EQ( offset, data.find( early ) );

	// Find the last 20 bytes of the file.
	std::string final = data.substr( data.size() - 20 );
	ASSERT_TRUE(ResourceTools::FindMatchingChunk( final, introMovieFilePath, offset ) );
	ASSERT_EQ( offset, data.size() - 20 );
}

TEST_F( ResourceToolsTest, CountMatchingChunks )
{
	const char* testDataPathStr = std::getenv( "TEST_DATA_PATH" );
	ASSERT_TRUE( testDataPathStr );
	std::filesystem::path testDataPath(testDataPathStr);
	std::filesystem::path introMovieFilePath = testDataPath / "Patch" / "previousBuildResources" / "introMoviePrefixed.txt";
	std::filesystem::path introMoviePatchedFilePath = testDataPath / "Patch" / "nextBuildResources" / "introMoviePrefixed.txt";

	const size_t CHUNK_SIZE{500};
	const size_t PREFIX_SIZE{308};
	ASSERT_EQ( ResourceTools::CountMatchingChunks( introMovieFilePath, 0, introMoviePatchedFilePath, 0, CHUNK_SIZE ), 0 );
	ASSERT_EQ( ResourceTools::CountMatchingChunks( introMovieFilePath, 0, introMoviePatchedFilePath, PREFIX_SIZE, CHUNK_SIZE ), 19 );
	ASSERT_EQ( ResourceTools::CountMatchingChunks( introMovieFilePath, CHUNK_SIZE, introMoviePatchedFilePath, CHUNK_SIZE + PREFIX_SIZE, 500 ), 18 );
}

#if __APPLE__
TEST_F( ResourceToolsTest, CalculateBinaryOperationMacOS )
{
	// Expected values
	// 33279: Binaries, not just executables (No extension, .so, .pyd)
	// 33188: Basically everything else
	std::filesystem::path testDataPath = std::getenv( "TEST_DATA_PATH" );
	std::filesystem::path textFilePath = testDataPath / "resourcesOnBranch" / "introMovie.txt";
	std::filesystem::path nonexistantFilePath = testDataPath / "resourcesOnBranch" / "thisFileDoesNotExist.txt";
	ASSERT_EQ(33188, ResourceTools::CalculateBinaryOperation(textFilePath));
	ASSERT_EQ(0, ResourceTools::CalculateBinaryOperation(nonexistantFilePath));
}
#elif WIN32
TEST_F( ResourceToolsTest, CalculateBinaryOperationWindows )
{
	// Expected values
	// 33279: Executables (.exe, .bat, .cmd, .com) # See: update_st_mode_from_path Modules/posixmodule.c
	// 33206: Basically everything else (.dll, .pyd, .yaml, .txt etc. )
	std::filesystem::path testDataPath = std::getenv( "TEST_DATA_PATH" );
	std::filesystem::path textFilePath = testDataPath / "resourcesOnBranch" / "introMovie.txt";
	std::filesystem::path nonexistantFilePath = testDataPath / "resourcesOnBranch" / "thisFileDoesNotExist.txt";
	std::filesystem::path binaryFilePath = std::filesystem::temp_directory_path() / "CarbonResources" /  "binary.exe";
	std::ofstream binaryFile(binaryFilePath);
	binaryFile.close();
	ASSERT_EQ(33279, ResourceTools::CalculateBinaryOperation(binaryFilePath));
	ASSERT_EQ(33206, ResourceTools::CalculateBinaryOperation(textFilePath));
	ASSERT_EQ(0, ResourceTools::CalculateBinaryOperation(nonexistantFilePath));
}
#endif

TEST_F( ResourceToolsTest, GzipStreams )
{
	std::string outbuffer;
	ResourceTools::GzipCompressionStream stream(&outbuffer);
	stream.Start();
	ResourceTools::FileDataStreamIn fileStreamIn( 50 );
	std::filesystem::path testDataPath = std::getenv( "TEST_DATA_PATH" );
	std::filesystem::path testFile = testDataPath / "resourcesOnBranch" / "introMovie.txt";
	fileStreamIn.StartRead(testFile);
	ResourceTools::Md5ChecksumStream originalMd5Stream;

	while (!fileStreamIn.IsFinished())
	{
		std::string fileData;
		ASSERT_TRUE(fileStreamIn >> fileData);
		std::string compressedData;
		originalMd5Stream << fileData;

		ASSERT_TRUE( stream << &fileData );
	}
	ASSERT_TRUE( stream.Finish() );


	std::string uncompressedData;
	ResourceTools::GzipDecompressionStream decompressionStream(&uncompressedData);
	decompressionStream.Start();
	EXPECT_TRUE( decompressionStream << &outbuffer );
	EXPECT_TRUE( decompressionStream.Finish() );

	std::string originalData;
	ResourceTools::GetLocalFileData( testFile, originalData );

	std::string originalChecksum;
	EXPECT_TRUE( originalMd5Stream.FinishAndRetrieve( originalChecksum ) );

	ResourceTools::Md5ChecksumStream uncompressedMd5Stream;
	uncompressedMd5Stream << uncompressedData;
	std::string uncompressedChecksum;
	EXPECT_TRUE( uncompressedMd5Stream.FinishAndRetrieve( uncompressedChecksum ) );
	EXPECT_EQ( uncompressedChecksum, originalChecksum );
}
