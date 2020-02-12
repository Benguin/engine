/**
 * @file
 */

#include "core/tests/AbstractTest.h"
#include "util/IncludeUtil.h"
#include "core/io/Filesystem.h"
#include <SDL_platform.h>

class IncludeUtilTest : public core::AbstractTest {
};

TEST_F(IncludeUtilTest, testInclude) {
	std::vector<core::String> includedFiles;
	std::vector<core::String> includeDirs { "." };
	const core::String src = io::filesystem()->load("main.h");
	EXPECT_FALSE(src.empty());
	std::pair<core::String, bool> retIncludes = util::handleIncludes(src, includeDirs, &includedFiles);
	EXPECT_TRUE(retIncludes.second);
	EXPECT_EQ(2u, includedFiles.size());
	EXPECT_EQ("#error \"one\"\n#include \"two.h\"\n\n#error \"two\"\n\n", retIncludes.first);
	retIncludes = util::handleIncludes(retIncludes.first, includeDirs, &includedFiles);
	EXPECT_TRUE(retIncludes.second);
	EXPECT_EQ(3u, includedFiles.size());
	EXPECT_EQ("#error \"one\"\n#error \"two\"\n\n\n#error \"two\"\n\n", retIncludes.first);
}
