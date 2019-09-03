#include <cppunit/extensions/HelperMacros.h>

#include <SmartFactory.h>

class BasicCase : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( BasicCase );
	CPPUNIT_TEST( MyTest );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();

protected:
	void MyTest();

	SmartFactory * sf;
};

CPPUNIT_TEST_SUITE_REGISTRATION( BasicCase );

void BasicCase::setUp()
{
	sf = new SmartFactory("test_factory");
}

void BasicCase::MyTest()
{
	float fnum = 2.00001f;
	CPPUNIT_ASSERT_DOUBLES_EQUAL( fnum, 2.0f, 0.0005 );

	std::string pattern_string("%%d pattern");
	std::string test_string("test pattern");
	std::string replace_string("test");

	std::string output_string = SmartFactory::placeholder(pattern_string, "%%d", replace_string);
	CPPUNIT_ASSERT_EQUAL(output_string, test_string);
}
