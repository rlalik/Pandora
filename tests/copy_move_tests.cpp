#include <cppunit/extensions/HelperMacros.h>

#include <SmartFactory.h>

class CopyMoveCase : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE( CopyMoveCase );
    CPPUNIT_TEST( testEquality );
    CPPUNIT_TEST_SUITE_END();

public:
<<<<<<< HEAD
	void setUp();
	void tearDown();
=======
    void setUp() {
            sf1 = new SmartFactory("test_factory_1");
            sf2 = new SmartFactory("test_factory_2");
    }
>>>>>>> ec3a463 (Update tests)

    void testEquality() {
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

private:
    SmartFactory * sf1;
    SmartFactory * sf2;
};

CPPUNIT_TEST_SUITE_REGISTRATION( CopyMoveCase );
<<<<<<< HEAD

void CopyMoveCase::setUp()
{
	sf1 = new SmartFactory("test_factory_1");
	sf2 = new SmartFactory("test_factory_2");
}

void CopyMoveCase::tearDown()
{
	delete sf1;
	delete sf2;
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
=======
>>>>>>> ec3a463 (Update tests)
