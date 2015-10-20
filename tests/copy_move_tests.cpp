#include <cppunit/extensions/HelperMacros.h>

#include <SmartFactory.h>

class CopyMoveCase : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CopyMoveCase );
	CPPUNIT_TEST( MyTest );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();

protected:
	void MyTest();

	SmartFactory * sf1;
	SmartFactory * sf2;
};

CPPUNIT_TEST_SUITE_REGISTRATION( CopyMoveCase );

void CopyMoveCase::setUp()
{
	sf1 = new SmartFactory("test_factory_1");
	sf2 = new SmartFactory("test_factory_2");
}

void CopyMoveCase::MyTest()
{
	TH1I * h1 = sf1->RegTH1<TH1I>("h1", "h1", 10, 0, 10, 1);

	h1->Fill(3);

	CPPUNIT_ASSERT_EQUAL( h1->GetBinContent(4), 1.0 );
	CPPUNIT_ASSERT_EQUAL( h1->GetBinContent(6), 0.0 );

	*sf2 = *sf1;

	TH1I * h2 = (TH1I*)sf2->getObject("h1");

	h2->Fill(5);

	CPPUNIT_ASSERT_EQUAL( h1->GetBinContent(4), 1.0 );
	CPPUNIT_ASSERT_EQUAL( h1->GetBinContent(6), 0.0 );
	CPPUNIT_ASSERT_EQUAL( h2->GetBinContent(4), 1.0 );
	CPPUNIT_ASSERT_EQUAL( h2->GetBinContent(6), 1.0 );

	CPPUNIT_ASSERT_EQUAL( 0, sf2->findIndex(h2) );
	CPPUNIT_ASSERT_EQUAL( -1, sf2->findIndex(h1) );

	CPPUNIT_ASSERT(sf2->name() != sf1->name());
}
