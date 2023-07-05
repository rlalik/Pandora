#include <gtest/gtest.h>

#include <pandora.hpp>

TEST(BasicTests, Placeholders)
{
    pandora::pandora* sf = new pandora::pandora("test_factory");

    std::string pattern_string("%%d pattern");
    std::string test_string("test pattern");
    std::string replace_string("test");

    std::string output_string = pandora::pandora::replace_placeholder(pattern_string, "%%d", replace_string);

    EXPECT_EQ(output_string, test_string);

    delete sf;
}
