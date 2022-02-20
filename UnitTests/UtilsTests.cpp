#include <GameCore/Utils.h>

#include <unordered_map>

#include "gtest/gtest.h"

TEST(UtilsTests, MakeFilenameSafeString_Beginning)
{
    char str[] = "\xec\xf5\xe8\xf1\xf8WOOZBAR";

    std::string const safeFilename = Utils::MakeFilenameSafeString(std::string(str));
    EXPECT_EQ(safeFilename, "WOOZBAR");
}

TEST(UtilsTests, MakeFilenameSafeString_Middle)
{
    char str[] = "FOO\xec\xf5\xe8\xf1\xf8ZBAR";

    std::string const safeFilename = Utils::MakeFilenameSafeString(std::string(str));
    EXPECT_EQ(safeFilename, "FOOZBAR");
}

TEST(UtilsTests, MakeFilenameSafeString_End)
{
    char str[] = "FOOZBAR\xec\xf5\xe8\xf1\xf8";

    std::string const safeFilename = Utils::MakeFilenameSafeString(std::string(str));
    EXPECT_EQ(safeFilename, "FOOZBAR");
}

TEST(UtilsTests, MakeFilenameSafeString_FilenameChars)
{
    char str[] = "FOO\\BAR/Z:";

    std::string const safeFilename = Utils::MakeFilenameSafeString(std::string(str));
    EXPECT_EQ(safeFilename, "FOOBARZ");
}

TEST(UtilsTests, MakeFilenameSafeString_BecomesEmpty)
{
    char str[] = "\xec\xf5\xe8\xf1\xf8";

    std::string const safeFilename = Utils::MakeFilenameSafeString(std::string(str));
    EXPECT_EQ(safeFilename, "");
}

TEST(UtilsTests, MakeFilenameSafeString_AlreadySafe)
{
    char str[] = "Foo Bar Hello";

    std::string const safeFilename = Utils::MakeFilenameSafeString(std::string(str));
    EXPECT_EQ(safeFilename, std::string(str));
}

class IsFileUnderDirectoryTest : public testing::TestWithParam<std::tuple<std::string, std::string, bool>>
{
public:
    virtual void SetUp() {}
    virtual void TearDown() {}
};

INSTANTIATE_TEST_SUITE_P(
    IsFileUnderDirectoryTests,
    IsFileUnderDirectoryTest,
    ::testing::Values( // Dir, File, Result
        std::make_tuple("C:\\", "C:\\foo\\zorro\\blah", true),
        std::make_tuple("C:\\foo", "C:\\foo\\zorro\\blah", true),
        std::make_tuple("C:\\foo\\zorro", "C:\\foo\\zorro\\blah", true),
        std::make_tuple("C:\\foo\\zorro\\blah", "C:\\foo\\zorro\\blah", true),

        std::make_tuple("C:\\foo\\zorro\\blah\\krok", "C:\\", false),
        std::make_tuple("C:\\foo\\zorro\\blah\\krok", "C:\\foo", false),
        std::make_tuple("C:\\foo\\zorro\\blah\\krok", "C:\\foo\\zorro", false),
        std::make_tuple("C:\\foo\\zorro\\blah\\krok", "C:\\foo\\zorro\\blah", false),

        std::make_tuple("/", "/foo/zorro/blah", true),
        std::make_tuple("/foo", "/foo/zorro/blah", true),
        std::make_tuple("/foo/zorro", "/foo/zorro/blah", true),
        std::make_tuple("/foo/zorro/blah", "/foo/zorro/blah", true),

        std::make_tuple("/foo/zorro/blah/krok", "/", false),
        std::make_tuple("/foo/zorro/blah/krok", "/foo", false),
        std::make_tuple("/foo/zorro/blah/krok", "/foo/zorro", false),
        std::make_tuple("/foo/zorro/blah/krok", "/foo/zorro/blah", false)
    ));

TEST_P(IsFileUnderDirectoryTest, BasicCases)
{
    auto const directoryPath = std::filesystem::path(std::get<0>(GetParam()));
    auto const filePath = std::filesystem::path(std::get<1>(GetParam()));

    bool const result = Utils::IsFileUnderDirectory(filePath, directoryPath);
    EXPECT_EQ(result, std::get<2>(GetParam()));
}