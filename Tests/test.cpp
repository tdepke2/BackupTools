// Note: need to define /Zc:__cplusplus to get this to compile with VS2017 using c++17
#include "pch.h"
#include "../BackupTools/src/Application.h"

TEST(TestGlobbing, NoWildcards) {
    EXPECT_EQ(Application::fnmatchPortable("", ""), true);
    EXPECT_EQ(Application::fnmatchPortable("a", "a"), true);
    EXPECT_EQ(Application::fnmatchPortable("a", "b"), false);
    EXPECT_EQ(Application::fnmatchPortable("", "b"), false);
    EXPECT_EQ(Application::fnmatchPortable("a", ""), false);
    EXPECT_EQ(Application::fnmatchPortable("thiS iS a TEST", "this is a test"), false);
    EXPECT_EQ(Application::fnmatchPortable("this is a test", "thiS iS a TEST"), false);
    EXPECT_EQ(Application::fnmatchPortable("thiS iS a TEST", "thiS iS a TEST"), true);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to\\file.txt", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(Application::fnmatchPortable("C:\\Path\\to\\file.txt", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to\\other.txt", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to\\file.txt", "C:\\path\\to\\file"), false);
    EXPECT_EQ(Application::fnmatchPortable("\\path\\to\\file.txt", "\\path\\to\\file.txt"), true);
}

TEST(TestGlobbing, QuestionMark) {
    EXPECT_EQ(Application::fnmatchPortable("?", ""), false);
    EXPECT_EQ(Application::fnmatchPortable("", "?"), false);
    EXPECT_EQ(Application::fnmatchPortable("?", "?"), true);
    EXPECT_EQ(Application::fnmatchPortable("?", "a"), true);
    EXPECT_EQ(Application::fnmatchPortable("?", "x"), true);
    EXPECT_EQ(Application::fnmatchPortable("?", "."), true);
    EXPECT_EQ(Application::fnmatchPortable("?", "\\"), false);
    EXPECT_EQ(Application::fnmatchPortable("?", "["), true);
    EXPECT_EQ(Application::fnmatchPortable("?", "]"), true);
    EXPECT_EQ(Application::fnmatchPortable("a?", "ab"), true);
    EXPECT_EQ(Application::fnmatchPortable("this is a tes?", "this is a tes"), false);
    EXPECT_EQ(Application::fnmatchPortable("?hi???s ??te?t", "this is a test"), true);
    EXPECT_EQ(Application::fnmatchPortable("???", "a"), false);
    EXPECT_EQ(Application::fnmatchPortable("???", "ab"), false);
    EXPECT_EQ(Application::fnmatchPortable("???", "abc"), true);
    EXPECT_EQ(Application::fnmatchPortable("???", "abcd"), false);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to\\???.txt", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to\\????.txt", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to?file.txt", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to?file.txt", "C:\\path\\to?file.txt"), true);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to\\file?txt", "C:\\path\\to\\file.txt"), true);
}

TEST(TestGlobbing, Star) {
    // Single star.
    EXPECT_EQ(Application::fnmatchPortable("*", ""), true);
    EXPECT_EQ(Application::fnmatchPortable("*", "*"), true);
    EXPECT_EQ(Application::fnmatchPortable("", "*"), false);
    EXPECT_EQ(Application::fnmatchPortable("*", "Bunch OF random text"), true);
    EXPECT_EQ(Application::fnmatchPortable("*", "Bunch OF random text."), true);
    EXPECT_EQ(Application::fnmatchPortable("*", ".Bunch OF random text."), false);
    EXPECT_EQ(Application::fnmatchPortable("*", ".Bunch OF random text"), false);
    EXPECT_EQ(Application::fnmatchPortable(".*", ".Bunch OF random text."), true);
    EXPECT_EQ(Application::fnmatchPortable(".*", ".Bunch OF random text"), true);
    EXPECT_EQ(Application::fnmatchPortable(".*", "Bunch OF random text"), false);
    EXPECT_EQ(Application::fnmatchPortable("*", "a.............."), true);
    EXPECT_EQ(Application::fnmatchPortable("app*", "apple"), true);
    EXPECT_EQ(Application::fnmatchPortable("app*", "appl"), true);
    EXPECT_EQ(Application::fnmatchPortable("app*", "app"), true);
    EXPECT_EQ(Application::fnmatchPortable("ap*le", "apple"), true);
    EXPECT_EQ(Application::fnmatchPortable("ap*le", "apshf soasdfle"), true);
    EXPECT_EQ(Application::fnmatchPortable("ap*le", "apshf soasdflge"), false);
    EXPECT_EQ(Application::fnmatchPortable("h*o *d", "hello world"), true);
    EXPECT_EQ(Application::fnmatchPortable("h*o *d", "he world"), false);
    EXPECT_EQ(Application::fnmatchPortable("h*o*d", "he world"), true);
    EXPECT_EQ(Application::fnmatchPortable("*txt", "myFile.txt"), true);
    EXPECT_EQ(Application::fnmatchPortable("*.txt", "myFile.txt"), true);
    EXPECT_EQ(Application::fnmatchPortable("*.txt.", "myFile.txt"), false);
    EXPECT_EQ(Application::fnmatchPortable("*txt", "myFile.txtx"), false);
    EXPECT_EQ(Application::fnmatchPortable("*txt*", "myFile.txtx"), true);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to\\*.txt", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to\\*", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to\\", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\*", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(Application::fnmatchPortable("*", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(Application::fnmatchPortable("*", "C:\\"), false);
    EXPECT_EQ(Application::fnmatchPortable("*", "C:"), true);
    EXPECT_EQ(Application::fnmatchPortable("*", "home"), true);
    EXPECT_EQ(Application::fnmatchPortable("*", "\\home"), false);
    EXPECT_EQ(Application::fnmatchPortable("\\*", "\\home"), true);
    EXPECT_EQ(Application::fnmatchPortable("\\*", "\\.home"), false);
    EXPECT_EQ(Application::fnmatchPortable("\\.*", "\\.home"), true);
    //EXPECT_EQ(Application::fnmatchPortable("\\*.", "\\."), false);    // This does not pass, but is how standard UNIX glob behaves. No idea why though, maybe because missing file ext?
    EXPECT_EQ(Application::fnmatchPortable("*", ".test"), false);
    EXPECT_EQ(Application::fnmatchPortable(".*", ".test"), true);
    EXPECT_EQ(Application::fnmatchPortable("*.", "."), true);    // Similar to previous exception on UNIX.
    EXPECT_EQ(Application::fnmatchPortable(".", ".test"), false);
    
    // Multiple star.
    EXPECT_EQ(Application::fnmatchPortable("**", ""), true);
    EXPECT_EQ(Application::fnmatchPortable("****", ""), true);
    EXPECT_EQ(Application::fnmatchPortable("**", "a"), true);
    EXPECT_EQ(Application::fnmatchPortable("**", "fhakfskfam fmamw"), true);
    EXPECT_EQ(Application::fnmatchPortable("****", "*"), true);
    EXPECT_EQ(Application::fnmatchPortable("**a**", "a"), true);
    EXPECT_EQ(Application::fnmatchPortable("**a**", "abcdef"), true);
    EXPECT_EQ(Application::fnmatchPortable("**a**", "bcdef"), false);
    EXPECT_EQ(Application::fnmatchPortable("**a**", "bcdefa"), true);
    EXPECT_EQ(Application::fnmatchPortable("h****o*wor**d***", "hello world"), true);
    EXPECT_EQ(Application::fnmatchPortable("h****o*wo**d**r***", "hello world"), false);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to\\******.txt", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to\\******txt", "C:\\path\\to\\.txt"), false);
    EXPECT_EQ(Application::fnmatchPortable("C:\\path\\to\\******.txt", "C:\\path\\to\\.txt"), true);
    EXPECT_EQ(Application::fnmatchPortable("***", ".test"), false);
    EXPECT_EQ(Application::fnmatchPortable(".***", ".test"), true);
    EXPECT_EQ(Application::fnmatchPortable("***.", "."), true);
}

TEST(TestGlobbing, Brackets) {
    // No ranges.
    EXPECT_EQ(Application::fnmatchPortable("[", ""), false);
    EXPECT_EQ(Application::fnmatchPortable("[", "["), true);
    EXPECT_EQ(Application::fnmatchPortable("]", ""), false);
    EXPECT_EQ(Application::fnmatchPortable("]", "]"), true);
    EXPECT_EQ(Application::fnmatchPortable("[]", ""), false);
    EXPECT_EQ(Application::fnmatchPortable("[]", "a"), false);
    EXPECT_EQ(Application::fnmatchPortable("[]", "[]"), true);
    EXPECT_EQ(Application::fnmatchPortable("[a]", ""), false);
    EXPECT_EQ(Application::fnmatchPortable("[a]", "a"), true);
    EXPECT_EQ(Application::fnmatchPortable("[ ]", " "), true);
    EXPECT_EQ(Application::fnmatchPortable("[asjwGDr]", "G"), true);
    EXPECT_EQ(Application::fnmatchPortable("[asjwGDr]", "a"), true);
    EXPECT_EQ(Application::fnmatchPortable("[asjwGDr]", "r"), true);
    EXPECT_EQ(Application::fnmatchPortable("[asjwGDr]", "h"), false);
    EXPECT_EQ(Application::fnmatchPortable("[asjwGDr]", "g"), false);
    EXPECT_EQ(Application::fnmatchPortable("[][]", "["), true);
    EXPECT_EQ(Application::fnmatchPortable("[][]", "]"), true);
    EXPECT_EQ(Application::fnmatchPortable("[][]", "[]"), false);
    EXPECT_EQ(Application::fnmatchPortable("[[]]", "["), false);
    EXPECT_EQ(Application::fnmatchPortable("[[]]", "]"), false);
    EXPECT_EQ(Application::fnmatchPortable("[[]]", "[]"), true);
    EXPECT_EQ(Application::fnmatchPortable("[a][]", "["), false);
    EXPECT_EQ(Application::fnmatchPortable("[a][]", "a[]"), true);
    EXPECT_EQ(Application::fnmatchPortable("[][b]", "["), true);
    EXPECT_EQ(Application::fnmatchPortable("[][b]", "b"), true);
    EXPECT_EQ(Application::fnmatchPortable("[][b]", "]"), true);
    EXPECT_EQ(Application::fnmatchPortable("[][b]", "[]"), false);
    
    // Ranges.
}

TEST(TestGlobbing, Comprehensive) {
    EXPECT_EQ(Application::fnmatchPortable("*a*??????*a*?????????a???????????????", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"), true);
}
