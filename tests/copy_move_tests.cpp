#include <gtest/gtest.h>

#include <pandora.hpp>

#include <TH1.h>

TEST(BasicTests, CopyMoveCase)
{
    pandora::pandora* sf1 = new pandora::pandora("test_factory_1");
    pandora::pandora* sf2 = new pandora::pandora("test_factory_2");

    TH1I* h1 = sf1->reg_hist<TH1I>("h1", "h1", 10, 0, 10);
    h1->Sumw2();
    h1->Fill(3);

    *sf2 = *sf1;

    TH1I* h2 = (TH1I*)sf2->get_object("h1");

    h2->Fill(5);

    EXPECT_FLOAT_EQ(h1->GetBinContent(4), 1.0);
    EXPECT_FLOAT_EQ(h1->GetBinContent(6), 0.0);
    EXPECT_FLOAT_EQ(h2->GetBinContent(4), 1.0);
    EXPECT_FLOAT_EQ(h2->GetBinContent(6), 1.0);

    EXPECT_EQ(0, sf2->find_index(h2));
    EXPECT_EQ(-1, sf2->find_index(h1));

    EXPECT_TRUE(sf2->name() != sf1->name());

    delete sf1;
    delete sf2;
}
