#include <GameCore/FileSystem.h>
#include <GameCore/MemoryStreams.h>

#include <map>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

::testing::AssertionResult ApproxEquals(float a, float b, float tolerance);

class TestFileSystem : public IFileSystem
{
public:

    using FileMap = std::map<std::filesystem::path, std::shared_ptr<memory_streambuf>>;

    TestFileSystem()
        : mFileMap()
    {}

    FileMap & GetFileMap()
    {
        return mFileMap;
    }

    ///////////////////////////////////////////////////////////
    // IFileSystem
    ///////////////////////////////////////////////////////////

    void EnsureDirectoryExists(std::filesystem::path const & /*directoryPath*/) override
    {
        // Nop
    }

    std::shared_ptr<std::istream> OpenInputStream(std::filesystem::path const & filePath) override
    {
        auto it = mFileMap.find(filePath);
        if (it != mFileMap.end())
        {
            return std::make_shared<std::istream>(it->second.get());
        }
        else
        {
            return std::shared_ptr<std::istream>();
        }
    }

    std::shared_ptr<std::ostream> OpenOutputStream(std::filesystem::path const & filePath) override
    {
        auto streamBuf = std::make_shared<memory_streambuf>();        
        mFileMap[filePath] = streamBuf;

        return std::make_shared<std::ostream>(streamBuf.get());
    }

    std::vector<std::filesystem::path> ListFiles(std::filesystem::path const & directoryPath) override
    {
        std::vector<std::filesystem::path> filePaths;

        for (auto const & kv : mFileMap)
        {
            if (kv.first.compare(directoryPath) > 0)
                filePaths.push_back(kv.first);
        }

        return filePaths;
    }

    void DeleteFile(std::filesystem::path const & filePath) override
    {
        auto it = mFileMap.find(filePath);
        if (it == mFileMap.end())
            throw std::logic_error("File path '" + filePath.string() + "' does not exist in test file system");

        mFileMap.erase(it);
    }

private:

    FileMap mFileMap;
};

class MockFileSystem : public IFileSystem 
{
public:

    MOCK_METHOD1(EnsureDirectoryExists, void(std::filesystem::path const & directoryPath));
    MOCK_METHOD1(OpenOutputStream, std::shared_ptr<std::ostream>(std::filesystem::path const & filePath));
    MOCK_METHOD1(OpenInputStream, std::shared_ptr<std::istream>(std::filesystem::path const & filePath));
    MOCK_METHOD1(ListFiles, std::vector<std::filesystem::path>(std::filesystem::path const & directoryPath));
    MOCK_METHOD1(DeleteFile, void(std::filesystem::path const & filePath));
};