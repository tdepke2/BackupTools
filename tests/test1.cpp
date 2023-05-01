// Note: need to define /Zc:__cplusplus to get this to compile with VS2017 using c++17
#include "BackupTools/ArgumentParser.h"
#include "BackupTools/FileHandler.h"
#include <gtest/gtest.h>
#include <string>
#include <cstring>

// ****************************************************************************
// * TestGlobbing                                                             *
// ****************************************************************************

TEST(TestGlobbing, NoWildcards) {
    FileHandler::pathSeparator = '\\';
    FileHandler::globMatchesHiddenFiles = false;
    
    EXPECT_EQ(FileHandler::fnmatchPortable("", ""), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("a", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("a", "b"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("", "b"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("a", ""), false);
    EXPECT_EQ(FileHandler::fnmatchPortable(" aslkwas  aowdsaknfal ", " awijalskd awoidwa "), false);
    EXPECT_EQ(FileHandler::fnmatchPortable(" aslkwas  aowdsaknfal ", " aslkwas  aowdsaknfal "), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("`~1!2@3#4$5%6^7&89(0)-_=+{}\\|;:\'\",<.>/", "`~1!2@3#4$5%6^7&89(0)-_=+{}\\|;:\'\",<./"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("`~1!2@3#4$5%6^7&89(0)-_=+{}\\|;:\'\",<.>/", "`~1!2@3#4$5%6^7&89(0)-_=+{}\\|;:\'\",<.>/"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("thiS iS a TEST", "this is a test"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("this is a test", "thiS iS a TEST"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("thiS iS a TEST", "thiS iS a TEST"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\file.txt", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\Path\\to\\file.txt", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\other.txt", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\file.txt", "C:\\path\\to\\file"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\path\\to\\file.txt", "\\path\\to\\file.txt"), true);
}

TEST(TestGlobbing, CommonCaseTests) {
    FileHandler::pathSeparator = '\\';
    FileHandler::globMatchesHiddenFiles = false;
    
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\file.txt", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\file.txt", "D:\\path\\to\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file.txt", "C:\\really\\long\\path\\to\\a\\different kind of\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file.txt", "C:\\really\\long\\path\\to\\a\\differentkind of\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file.txt", "C:\\really\\long\\path\\to\\a\\different kind of\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file2.txt", "C:\\really\\long\\path\\to\\a\\different kind of\\file2.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file2.txt", "C:\\really\\long\\path\\to\\a\\different kind of\\file3.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file3.txt", "C:\\really\\long\\path\\to\\a\\different kind of\\file3.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file4.txt", "C:\\really\\long\\path\\to\\a\\different kind of\\file5.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file4.txt", "C:\\really\\long\\path\\to\\a\\different kind of\\file4.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file.txt", "C:\\"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file.txt", "C:\\really"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file.txt", "C:\\really\\long"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file.txt", "C:\\really\\long\\path"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file.txt", "C:\\really\\long\\path\\to"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file.txt", "C:\\really\\long\\path\\to\\a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file.txt", "C:\\really\\long\\path\\to\\a\\different kind of"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\really\\long\\path\\to\\a\\different kind of\\file.txt", "C:\\really\\long\\path\\to\\a\\different kind of\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home", "\\home"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home", "\\usr"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home", "\\local"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account", "\\home\\lost+found"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account", "\\home\\user account"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account", "\\home\\temp"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account\\documents", "\\home\\user account\\stuff"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account\\documents", "\\home\\user account\\music"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account\\documents", "\\home\\user account\\Open Office"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account\\documents", "\\home\\user account\\documents"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account\\documents\\*.bin", "\\home\\user account\\documents\\new text doc.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account\\documents\\*.bin", "\\home\\user account\\documents\\file"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account\\documents\\*.bin", "\\home\\user account\\documents\\.bin"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account\\documents\\*.bin", "\\home\\user account\\documents\\.hidden"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account\\documents\\*.bin", "\\home\\user account\\documents\\data.bin"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account\\documents\\*.bin", "\\home\\user account\\documents\\."), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\home\\user account\\documents\\*.bin", "\\home\\user account\\documents\\.."), false);
}

TEST(TestGlobbing, QuestionMark) {
    FileHandler::pathSeparator = '\\';
    FileHandler::globMatchesHiddenFiles = false;
    
    EXPECT_EQ(FileHandler::fnmatchPortable("?", ""), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("", "?"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("?", "?"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("?", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("?", "x"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("?", "."), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\?", "\\."), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("a?", "a."), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\a?\\", "\\a.\\"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("?", "\\"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("?", "["), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("?", "]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("a?", "a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("a?", "ab"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("this is a tes?", "this is a tes"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("?hi???s ??te?t", "this is a test"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("???", "a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("???", "ab"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("???", "abc"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("???", "abcd"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\???.txt", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\????.txt", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to?file.txt", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to?file.txt", "C:\\path\\to?file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\file?txt", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\?file", "C:\\path\\to\\.file"), false);
}

TEST(TestGlobbing, Star) {
    FileHandler::pathSeparator = '\\';
    FileHandler::globMatchesHiddenFiles = false;
    
    // Single star.
    EXPECT_EQ(FileHandler::fnmatchPortable("*", ""), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", "*"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("", "*"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("a*", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("a*b", "ab"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("a*b", "acb"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("a*b", "abc"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("a*b*", "abc"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", "Bunch OF random text"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", "Bunch OF random text."), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", ".Bunch OF random text."), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", ".Bunch OF random text"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable(".*", ".Bunch OF random text."), true);
    EXPECT_EQ(FileHandler::fnmatchPortable(".*", ".Bunch OF random text"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable(".*", "Bunch OF random text"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable(".*", "Bunch OF random text."), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*.*", "Bunch OF random text"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*.*", "Bunch OF random text."), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*.*", "Bunch OF random.text"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*.*", ".Bunch OF random text"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", "a.............."), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("app*", "apple"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("app*", "appl"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("app*", "app"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("app*", "ap"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("ap*le", "apple"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("ap*le", "apshf soasdfle"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("ap*le", "apshf soasdflge"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("h*o *d", "hello world"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("h*o *d", "he world"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("h*o*d", "he world"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*txt", "myFile.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*.txt", "myFile.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*.txt.", "myFile.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*txt", "myFile.txtx"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*txt*", "myFile.txtx"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*.txt", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\*", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", "C:\\"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", "C:"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", "home"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", "\\home"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\*", "\\home"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\*", "\\.home"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\.*", "\\.home"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("\\*.", "\\."), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", ".test"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", "test"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable(".*", ".test"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable(".*", "test"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*.", "."), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*.", "a."), true);
    EXPECT_EQ(FileHandler::fnmatchPortable(".", ".test"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", "."), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", ".."), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", "test.dat"), true);
    
    // Multiple star.
    EXPECT_EQ(FileHandler::fnmatchPortable("**", ""), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("****", ""), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("**", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("**", "fhakfskfam fmamw"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("****", "*"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("**a**", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("**a**", "abcdef"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("**a**", "bcdef"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("**a**", "bcdefa"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("h****o*wor**d***", "hello world"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("h****o*wo**d**r***", "hello world"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\******.txt", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\******txt", "C:\\path\\to\\.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\******.txt", "C:\\path\\to\\.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\******txt", "C:\\path\\to\\txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("***", ".test"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable(".***", ".test"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("***.", "."), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("***.", ".."), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("***.", "a."), true);
}

TEST(TestGlobbing, Brackets) {
    FileHandler::pathSeparator = '\\';
    FileHandler::globMatchesHiddenFiles = false;
    
    // No ranges.
    EXPECT_EQ(FileHandler::fnmatchPortable("[", ""), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[", "["), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("]", ""), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("]", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("]", "]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]", ""), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]", "a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]", "ab"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]", "[]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]hello", ""), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]hello", "[]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]hello", "hello"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]hello", "[]hello"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a]", ""), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ ]", " "), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[asjwGDr]", "G"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[asjwGDr]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[asjwGDr]", "r"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[asjwGDr]", "h"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[asjwGDr]", "g"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[asjwGDr", "G"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[asjwGDr", "a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[asjwGDr", "r"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[asjwGDr", "[asjwGDr"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[asjwGDr", "asjwGDr"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[][]", "["), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[][]", "]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[][]", "[]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[[]]", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[[]]", "]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[[]]", "[]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[[]", "["), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[[]", "]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[[]", "[[]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]]", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]]", "]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]]", "[]]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a][]", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a][]", "a[]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[][b]", "["), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[][b]", "b"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[][b]", "]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[][b]", "[]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ab][cd]", "[ab][cd]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ab][cd]", "]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ab][cd]", "abcd"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ab][cd]", "ac"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ab][cd]", "ab"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ab][cd]", "bc"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ab][cd]", "bd"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ab][cd]", "ad"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ab][cd]", "cd"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ab][cd]", "da"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[[[[[[[[[", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[[[[[[[[[", "[[[[[[[[["), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("]]]]]]]]]", "]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("]]]]]]]]]", "]]]]]]]]]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a]", ""), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^a]", ""), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a]", "a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^a]", "a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a]", "b"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^a]", "b"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a!]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a^]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a!]", "!"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a^]", "^"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!^]", "^"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!^]", "g"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^!]", "!"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^!]", "g"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!asjwGDr]", "G"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!asjwGDr]", "a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!asjwGDr]", "r"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!asjwGDr]", "h"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!asjwGDr]", "g"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!asjwGDr", "h"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!asjwGDr", "[!asjwGDr"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!asjwGDr", "[asjwGDr"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\[f]ile.txt", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\[f]ile.txt", "C:\\path\\to\\aile.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\[!f]ile.txt", "C:\\path\\to\\aile.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\[file][file][file][file].txt", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\[file][file][file][file].txt", "C:\\path\\to\\life.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to[\\]file.txt", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\[f\\]ile.txt", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\[\\f]ile.txt", "C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\[f\\]ile.txt", "C:\\path\\to\\[f\\]ile.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\file[.]txt", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\file[.]txt", "C:\\path\\to\\filetxt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\[.]file", "C:\\path\\to\\.file"), true);    // The standard unix glob does not support this, but it seems like a good idea since brace expansion is not a feature of this function.
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\[!.]file", "C:\\path\\to\\.file"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\[!.]file", "C:\\path\\to\\afile"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[.]file", ".file"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[.]file", "file"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!.]file", ".file"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!.]file", "afile"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!", "!"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!", "[!"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^", "^"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^", "[^"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]", "!"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]", "]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]", "[!]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]x", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]x", "[!]x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]x", "!x"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]x", "^x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^]", "^"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^]", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^]", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^]", "]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^]", "[^]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^]x", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^]x", "[^]x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^]x", "^x"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^]x", "!x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]]", "!"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]]", "]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]]", "x"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]abcdef]", "!"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]abcdef]", "]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]abcdef]", "x"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]abcdef]", "a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]abcdef]", "f"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!!]", "[!!]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!!]", "!"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!!]", "^"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!!]", "]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^^]", "[^^]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^^]", "^"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^^]", "!"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^^]", "]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!][!]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!][!]", "!"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!][!]", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!][!]", "]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[![]!]", "[!]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[![]!]", "!!]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[![]!]", "a!]"), true);
    
    // Ranges.
    EXPECT_EQ(FileHandler::fnmatchPortable("[-", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-", "[-"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-a", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-a", "[-a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-", "[a-"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("-]", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("-]", "-]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("-a]", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("-a]", "-a]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("a-]", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("a-]", "a-]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-]", "[-]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-]", "[-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-]", "-]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-]", "-"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-a]", "-"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-a]", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-a]", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-a]", "]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-abc]", "-"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-abc]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-abc]", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-]", "-"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-]", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-]", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-]", "]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[abc-]", "-"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[abc-]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[abc-]", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-a-]", "-"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-a-]", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-a-]", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-a-]", "]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-abc-]", "-"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-abc-]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-abc-]", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[-abc-]", "x"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-a]", "[a-a]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-a]", "`"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-a]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-a]", "b"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-a]", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-a]", "]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-a]", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-b]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-b]", "b"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-b]", "c"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-z]", "`"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-z]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-z]", "b"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-z]", "h"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-z]", "m"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-z]", "v"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-z]", "z"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-z]", "{"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[z-a]", "z"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[z-a]", "a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[z-a]", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[z-a]", "[z-a]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[b-a]", "b"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[b-a]", "a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[b-a]", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[b-a]", "[b-a]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-cG-Ij]", "`"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-cG-Ij]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-cG-Ij]", "b"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-cG-Ij]", "c"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-cG-Ij]", "d"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-cG-Ij]", "g"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-cG-Ij]", "F"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-cG-Ij]", "G"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-cG-Ij]", "H"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-cG-Ij]", "I"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-cG-Ij]", "J"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-cG-Ij]", "j"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[a-cG-Ij]", "k"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ac-e]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ac-e]", "b"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ac-e]", "c"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ac-e]", "d"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ac-e]", "e"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[ac-e]", "f"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-z]", "`"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-z]", "a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-z]", "b"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-z]", "h"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-z]", "m"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-z]", "v"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-z]", "z"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-z]", "{"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!-z]", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!-z]", "z"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!-z]", "x"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!-z]", "!"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-]", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-]", "a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-]", "x"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-]", "!"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-a]", "a"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-a]", "b"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-a]", "-"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!a-a]", "!"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]]a-z]", "]a-z]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]]a-z]", "]b"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]a-z]", "]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]a-z]", "b"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[]a-z]", "A"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[[]a-z]", "[a-z]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[[]a-z]", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[[]a-z]", "b"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!-", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!-", "!"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!-", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!-", "[!-"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^-", "["), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^-", "^"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^-", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[^-", "[^-"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!-]", "-"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!-]", "!"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!-]", "x"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[![-]]", "a]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[![-]]", "[]"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]-[]", "["), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]-[]", "]"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]-[]", "a"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("[!]-[]", "-"), true);
}

TEST(TestGlobbing, Comprehensive) {
    FileHandler::pathSeparator = '\\';
    FileHandler::globMatchesHiddenFiles = false;
    
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*.???", "a.aaa"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*.???", "C:\\path\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*.???", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*.???", "C:\\path\\to\\file.cc"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*.???", "C:\\path\\to\\file.data"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*.???", "C:\\path\\to\\.hid"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("?[!!-@]*g[a-zA-Z0-9]", "xa!jam!gh"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("?[!!-@]*g[a-zA-Z0-9]", ".a!jam!gh"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("?[!!-@]*g[a-zA-Z0-9]", "x?!jam!gh"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("?[!!-@]*g[a-zA-Z0-9]", "xa!jam!h"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("?[!!-@]*g[a-zA-Z0-9]", "xa!jam!g@"), false);
    
    // These take some time, but are also very rare edge cases that do not need to be optimized.
    EXPECT_EQ(FileHandler::fnmatchPortable("*a*??????*a*?????????a???????????????", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*a*??????*a*?????????a???????????????", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabaaaaaaaaaaaaaaa"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("*a*??????*a*?????????a???????????????", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabaaaaaaaaaaaaaa"), true);
}

TEST(TestGlobbing, GlobMatchesHidden) {
    FileHandler::pathSeparator = '\\';
    FileHandler::globMatchesHiddenFiles = true;
    
    EXPECT_EQ(FileHandler::fnmatchPortable("?", "x"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("?", "."), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", "Bunch OF random text."), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("*", ".Bunch OF random text."), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*.???", "a.aaa"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*.???", "C:\\path\\file.txt"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*.???", "C:\\path\\to\\file.txt"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*.???", "C:\\path\\to\\file.cc"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*.???", "C:\\path\\to\\file.data"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("C:\\path\\to\\*.???", "C:\\path\\to\\.hid"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("?[!!-@]*g[a-zA-Z0-9]", "xa!jam!gh"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("?[!!-@]*g[a-zA-Z0-9]", ".a!jam!gh"), true);
    EXPECT_EQ(FileHandler::fnmatchPortable("?[!!-@]*g[a-zA-Z0-9]", "x?!jam!gh"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("?[!!-@]*g[a-zA-Z0-9]", "xa!jam!h"), false);
    EXPECT_EQ(FileHandler::fnmatchPortable("?[!!-@]*g[a-zA-Z0-9]", "xa!jam!g@"), false);
}

// ****************************************************************************
// * TestContainsWildcard                                                     *
// ****************************************************************************

TEST(TestContainsWildcard, Test1) {
    EXPECT_EQ(FileHandler::containsWildcard(""), false);
    EXPECT_EQ(FileHandler::containsWildcard("a"), false);
    EXPECT_EQ(FileHandler::containsWildcard("*"), true);
    EXPECT_EQ(FileHandler::containsWildcard("?"), true);
    EXPECT_EQ(FileHandler::containsWildcard("."), false);
    EXPECT_EQ(FileHandler::containsWildcard("["), false);
    EXPECT_EQ(FileHandler::containsWildcard("[*"), true);
    EXPECT_EQ(FileHandler::containsWildcard("[]"), false);
    EXPECT_EQ(FileHandler::containsWildcard("[]]"), true);
    EXPECT_EQ(FileHandler::containsWildcard("[[]"), true);
    EXPECT_EQ(FileHandler::containsWildcard("[a]"), true);
    EXPECT_EQ(FileHandler::containsWildcard("[abc "), false);
    EXPECT_EQ(FileHandler::containsWildcard("[abc] "), true);
    EXPECT_EQ(FileHandler::containsWildcard("?[!!-@]*g[a-zA-Z0-9]"), true);
    EXPECT_EQ(FileHandler::containsWildcard("`~1!2@3#4$5%6^7&89(0)-_=+{}\\|;:\'\",<.>/"), false);
    EXPECT_EQ(FileHandler::containsWildcard("C:\\path\\to\\file.txt"), false);
    EXPECT_EQ(FileHandler::containsWildcard("C:\\path\\to\\*.txt"), true);
}

// ****************************************************************************
// * TestArgumentParser                                                     *
// ****************************************************************************

TEST(TestArgumentParser, Test1) {
    ArgumentParser::OptionList options = {
        {'a', "add", ArgumentParser::RequiredArg, nullptr, 'a'},
        {'b', "", ArgumentParser::NoArg, nullptr, 'b'},
        {'\0', "thing", ArgumentParser::OptionalArg, nullptr, 't'}
    };
    ArgumentParser argParser(options);
    
    EXPECT_EQ(argParser.nextOption(), -1);
    EXPECT_TRUE(argParser.getOptionArg() == nullptr);
    
    {
        const char* argv[] = {
            "hello",
            "test",
            nullptr
        };
        argParser.setArguments(argv, 0);
        
        EXPECT_EQ(argParser.nextOption(), -1);
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 0);
        
        EXPECT_TRUE(std::strcmp(argv[0], "hello") == 0);
        EXPECT_TRUE(std::strcmp(argv[1], "test") == 0);
        EXPECT_TRUE(argv[2] == nullptr);
    }
    
    {
        const char* argv[] = {
            "--foobar",
            "test",
            "-cd",
            "a",
            "thing",
            nullptr
        };
        argParser.setArguments(argv, 0);
        
        std::string errorMessage = "";
        EXPECT_EQ(argParser.nextOption(&errorMessage), '?');
        EXPECT_EQ(errorMessage, "Unknown option --foobar");
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 1);
        
        errorMessage = "";
        EXPECT_EQ(argParser.nextOption(&errorMessage), '?');
        EXPECT_EQ(errorMessage, "Unknown option -c");
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 2);
        
        errorMessage = "";
        EXPECT_EQ(argParser.nextOption(&errorMessage), '?');
        EXPECT_EQ(errorMessage, "Unknown option -d");
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 3);
        
        errorMessage = "";
        EXPECT_EQ(argParser.nextOption(&errorMessage), -1);
        EXPECT_EQ(errorMessage, "");
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 2);
        
        EXPECT_TRUE(std::strcmp(argv[0], "--foobar") == 0);
        EXPECT_TRUE(std::strcmp(argv[1], "-cd") == 0);
        EXPECT_TRUE(std::strcmp(argv[2], "test") == 0);
        EXPECT_TRUE(std::strcmp(argv[3], "a") == 0);
        EXPECT_TRUE(std::strcmp(argv[4], "thing") == 0);
        EXPECT_TRUE(argv[5] == nullptr);
    }
    
    {
        const char* argv[] = {
            "--add",
            "-b",
            nullptr
        };
        argParser.setArguments(argv, 0);
        
        std::string errorMessage = "";
        EXPECT_EQ(argParser.nextOption(&errorMessage), ':');
        EXPECT_EQ(errorMessage, "Option --add requires an argument");
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 1);
        
        errorMessage = "";
        EXPECT_EQ(argParser.nextOption(&errorMessage), 'b');
        EXPECT_EQ(errorMessage, "");
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 2);
        
        errorMessage = "";
        EXPECT_EQ(argParser.nextOption(&errorMessage), -1);
        EXPECT_EQ(errorMessage, "");
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 2);
        
        EXPECT_TRUE(std::strcmp(argv[0], "--add") == 0);
        EXPECT_TRUE(std::strcmp(argv[1], "-b") == 0);
        EXPECT_TRUE(argv[2] == nullptr);
    }
    
    {
        const char* argv[] = {
            "--add",
            "b",
            "-b",
            nullptr
        };
        argParser.setArguments(argv, 0);
        
        EXPECT_EQ(argParser.nextOption(), 'a');
        EXPECT_TRUE(argParser.getOptionArg() == argv[1]);
        EXPECT_EQ(argParser.getIndex(), 2);
        
        EXPECT_EQ(argParser.nextOption(), 'b');
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 3);
        
        EXPECT_EQ(argParser.nextOption(), -1);
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 3);
        
        EXPECT_TRUE(std::strcmp(argv[0], "--add") == 0);
        EXPECT_TRUE(std::strcmp(argv[1], "b") == 0);
        EXPECT_TRUE(std::strcmp(argv[2], "-b") == 0);
        EXPECT_TRUE(argv[3] == nullptr);
    }
    
    {
        const char* argv[] = {
            "abc",
            "-b",
            "beans",
            "cool stuff",
            "--thing",
            "thingarg",
            nullptr
        };
        argParser.setArguments(argv, 0);
        
        EXPECT_EQ(argParser.nextOption(), 'b');
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 2);
        
        EXPECT_EQ(argParser.nextOption(), 't');
        EXPECT_TRUE(argParser.getOptionArg() == argv[5]);
        EXPECT_EQ(argParser.getIndex(), 6);
        
        EXPECT_EQ(argParser.nextOption(), -1);
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 3);
        
        EXPECT_TRUE(std::strcmp(argv[0], "-b") == 0);
        EXPECT_TRUE(std::strcmp(argv[1], "--thing") == 0);
        EXPECT_TRUE(std::strcmp(argv[2], "thingarg") == 0);
        EXPECT_TRUE(std::strcmp(argv[3], "abc") == 0);
        EXPECT_TRUE(std::strcmp(argv[4], "beans") == 0);
        EXPECT_TRUE(std::strcmp(argv[5], "cool stuff") == 0);
        EXPECT_TRUE(argv[6] == nullptr);
    }
    
    {
        const char* argv[] = {
            "--add",
            "9e7",
            "-a",
            "-12.567",
            nullptr
        };
        argParser.setArguments(argv, 0);
        
        EXPECT_EQ(argParser.nextOption(), 'a');
        EXPECT_TRUE(argParser.getOptionArg() == argv[1]);
        EXPECT_EQ(argParser.getIndex(), 2);
        
        EXPECT_EQ(argParser.nextOption(), 'a');
        EXPECT_TRUE(argParser.getOptionArg() == argv[3]);
        EXPECT_EQ(argParser.getIndex(), 4);
        
        EXPECT_EQ(argParser.nextOption(), -1);
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 4);
        
        EXPECT_TRUE(std::strcmp(argv[0], "--add") == 0);
        EXPECT_TRUE(std::strcmp(argv[1], "9e7") == 0);
        EXPECT_TRUE(std::strcmp(argv[2], "-a") == 0);
        EXPECT_TRUE(std::strcmp(argv[3], "-12.567") == 0);
        EXPECT_TRUE(argv[4] == nullptr);
    }
}

TEST(TestArgumentParser, Test2) {
    int helpFlag, listFlag;
    
    ArgumentParser::OptionList options = {
        {'h', "help", ArgumentParser::OptionalArg, &helpFlag, 'h'},
        {'x', "compress", ArgumentParser::RequiredArg, nullptr, 'x'},
        {'l', "list", ArgumentParser::NoArg, &listFlag, 4},
        {'\0', "print-verbose", ArgumentParser::NoArg, nullptr, 2},
        {'r', "", ArgumentParser::RequiredArg, nullptr, 'r'}
    };
    ArgumentParser argParser(options);
    
    helpFlag = 0;
    listFlag = 0;
    {
        const char* argv[] = {
            "bin/progname.exe",
            "-hlrx",
            "filename.txt",
            nullptr
        };
        argParser.setArguments(argv);
        
        EXPECT_EQ(argParser.nextOption(), 0);
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 1);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 0);
        
        EXPECT_EQ(argParser.nextOption(), 0);
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 1);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_EQ(argParser.nextOption(), ':');
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 1);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_EQ(argParser.nextOption(), 'x');
        EXPECT_TRUE(argParser.getOptionArg() == argv[2]);
        EXPECT_EQ(argParser.getIndex(), 3);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_EQ(argParser.nextOption(), -1);
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 3);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_TRUE(std::strcmp(argv[0], "bin/progname.exe") == 0);
        EXPECT_TRUE(std::strcmp(argv[1], "-hlrx") == 0);
        EXPECT_TRUE(std::strcmp(argv[2], "filename.txt") == 0);
        EXPECT_TRUE(argv[3] == nullptr);
    }
    
    helpFlag = 0;
    listFlag = 0;
    {
        const char* argv[] = {
            "bin/progname.exe",
            "-h",
            "-l",
            "-r",
            "-x",
            "filename.txt",
            nullptr
        };
        argParser.setArguments(argv);
        
        EXPECT_EQ(argParser.nextOption(), 0);
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 2);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 0);
        
        EXPECT_EQ(argParser.nextOption(), 0);
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 3);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_EQ(argParser.nextOption(), ':');
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 4);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_EQ(argParser.nextOption(), 'x');
        EXPECT_TRUE(argParser.getOptionArg() == argv[5]);
        EXPECT_EQ(argParser.getIndex(), 6);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_EQ(argParser.nextOption(), -1);
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 6);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_TRUE(std::strcmp(argv[0], "bin/progname.exe") == 0);
        EXPECT_TRUE(std::strcmp(argv[1], "-h") == 0);
        EXPECT_TRUE(std::strcmp(argv[2], "-l") == 0);
        EXPECT_TRUE(std::strcmp(argv[3], "-r") == 0);
        EXPECT_TRUE(std::strcmp(argv[4], "-x") == 0);
        EXPECT_TRUE(std::strcmp(argv[5], "filename.txt") == 0);
        EXPECT_TRUE(argv[6] == nullptr);
    }
    
    helpFlag = 0;
    listFlag = 0;
    {
        const char* argv[] = {
            "bin/progname.exe",
            "sample-text",
            "-h",
            "lrx",
            "-lr",
            "stuff",
            "--lr",
            "-x",
            "latest.log",
            "--print-verbose",
            "-",
            "--",
            "abc",
            nullptr
        };
        argParser.setArguments(argv);
        
        EXPECT_EQ(argParser.nextOption(), 0);
        EXPECT_TRUE(argParser.getOptionArg() == argv[3]);
        EXPECT_EQ(argParser.getIndex(), 4);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 0);
        
        EXPECT_EQ(argParser.nextOption(), 0);
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 4);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_EQ(argParser.nextOption(), 'r');
        EXPECT_TRUE(argParser.getOptionArg() == argv[5]);
        EXPECT_EQ(argParser.getIndex(), 6);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_EQ(argParser.nextOption(), '?');
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 7);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_EQ(argParser.nextOption(), 'x');
        EXPECT_TRUE(argParser.getOptionArg() == argv[8]);
        EXPECT_EQ(argParser.getIndex(), 9);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_EQ(argParser.nextOption(), 2);
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 10);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_EQ(argParser.nextOption(), -1);
        EXPECT_TRUE(argParser.getOptionArg() == nullptr);
        EXPECT_EQ(argParser.getIndex(), 9);
        EXPECT_EQ(helpFlag, 'h');
        EXPECT_EQ(listFlag, 4);
        
        EXPECT_TRUE(std::strcmp(argv[0], "bin/progname.exe") == 0);
        EXPECT_TRUE(std::strcmp(argv[1], "-h") == 0);
        EXPECT_TRUE(std::strcmp(argv[2], "lrx") == 0);
        EXPECT_TRUE(std::strcmp(argv[3], "-lr") == 0);
        EXPECT_TRUE(std::strcmp(argv[4], "stuff") == 0);
        EXPECT_TRUE(std::strcmp(argv[5], "--lr") == 0);
        EXPECT_TRUE(std::strcmp(argv[6], "-x") == 0);
        EXPECT_TRUE(std::strcmp(argv[7], "latest.log") == 0);
        EXPECT_TRUE(std::strcmp(argv[8], "--print-verbose") == 0);
        EXPECT_TRUE(std::strcmp(argv[9], "sample-text") == 0);
        EXPECT_TRUE(std::strcmp(argv[10], "-") == 0);
        EXPECT_TRUE(std::strcmp(argv[11], "--") == 0);
        EXPECT_TRUE(std::strcmp(argv[12], "abc") == 0);
        EXPECT_TRUE(argv[13] == nullptr);
    }
}
