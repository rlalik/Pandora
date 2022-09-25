#include <gtest/gtest.h>

#include <Pandora.h>

TEST(BasicTests, Placeholders)
{
    RT::Pandora* sf = new RT::Pandora("test_factory");

    std::string pattern_string("%%d pattern");
    std::string test_string("test pattern");
    std::string replace_string("test");

    std::string output_string = RT::Pandora::placeholder(pattern_string, "%%d", replace_string);

    EXPECT_EQ(output_string, test_string);

    delete sf;
}
