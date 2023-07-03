#include <gtest/gtest.h>

#include <Pandora.h>

#include <TH1.h>

TEST(BasicTests, CopyMoveCase)
{
    RT::Pandora* sf1 = new RT::Pandora("test_factory_1");
    RT::Pandora* sf2 = new RT::Pandora("test_factory_2");

    TH1I* h1 = sf1->RegHist<TH1I>("h1", "h1", 10, 0, 10);
    h1->Sumw2();
    h1->Fill(3);

    *sf2 = *sf1;

    TH1I* h2 = (TH1I*)sf2->getObject("h1");

    h2->Fill(5);

    EXPECT_FLOAT_EQ(h1->GetBinContent(4), 1.0);
    EXPECT_FLOAT_EQ(h1->GetBinContent(6), 0.0);
    EXPECT_FLOAT_EQ(h2->GetBinContent(4), 1.0);
    EXPECT_FLOAT_EQ(h2->GetBinContent(6), 1.0);

    EXPECT_EQ(0, sf2->findIndex(h2));
    EXPECT_EQ(-1, sf2->findIndex(h1));

    EXPECT_TRUE(sf2->name() != sf1->name());

    delete sf1;
    delete sf2;
}
