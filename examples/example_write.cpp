#include <pandora.hpp>

#include <TH1.h>

int main()
{
    // create box

    pandora::pandora* box = new pandora::pandora("box1");

    // fill with histograms
    char hname[100];
    for (int i = 0; i < 10; ++i)
    {
        sprintf(hname, "hist_%05d", i);
        TH1F* h = box->reg_hist<TH1F>(hname, "Histogram - loop", 100, -5, 5);
        h->FillRandom("gaus", 100000);
    }

    box->add_placeholder("{analysis}", "original_box");
    box->add_placeholder("{dir}", "original_directory");
    box->add_placeholder("{dir2}", "subdir");

    TH1F* hist1 = box->reg_hist<TH1F>("dir1/hist1", "Histogram 1", 100, -5, 5);
    TH1F* hist2 = box->reg_hist<TH1F>("hist2", "Histogram 2", 100, -5, 5);
    TH1F* hist3 = box->reg_hist<TH1F>("dir1/dir2/hist3", "Histogram 3", 100, -5, 5);
    TH1F* hist4 = box->reg_hist<TH1F>("{dir}/{dir2}/hist_{analysis}_placeholders", "Histogram 4", 100, -5, 5);

    box->add_placeholder("{analysis}", "renamed_box");
    box->add_placeholder("{dir}", "renamed_directory");

    hist1->FillRandom("gaus", 100000);
    hist2->FillRandom("gaus", 100000);
    hist3->FillRandom("gaus", 100000);
    hist4->FillRandom("gaus", 100000);

    // 	TCanvas * can = box->RegCanvas("can1", "Canvas", 800, 800, 4);

    // list objects
    box->list_registered_objects();

    // export box to file
    box->export_structure("example.root", true);
}
