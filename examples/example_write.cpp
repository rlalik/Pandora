#include "Pandora.h"

#include <TH1.h>

int main()
{
    // create box

    RT::Pandora* box = new RT::Pandora("box1");

    // fill with histograms
    char hname[100];
    for (int i = 0; i < 10; ++i)
    {
        sprintf(hname, "hist_%05d", i);
        TH1F* h = box->RegHist<TH1F>(hname, "Histogram - loop", 100, -5, 5);
        h->FillRandom("gaus", 100000);
    }

    TH1F* hist1 = box->RegHist<TH1F>("dir1/hist1", "Histogram 1", 100, -5, 5);
    TH1F* hist2 = box->RegHist<TH1F>("hist2", "Histogram 2", 100, -5, 5);
    TH1F* hist3 = box->RegHist<TH1F>("dir1/dir2/hist3", "Histogram 3", 100, -5, 5);
    TH1F* hist4 = box->RegHist<TH1F>("@@@d/hist_@@@a_placeholders", "Histogram 4", 100, -5, 5);

    box->rename("renamed_box");
    box->chdir("renamed_directory");

    hist1->FillRandom("gaus", 100000);
    hist2->FillRandom("gaus", 100000);
    hist3->FillRandom("gaus", 100000);

    // 	TCanvas * can = box->RegCanvas("can1", "Canvas", 800, 800, 4);

    // list objects
    box->listRegisteredObjects();

    // export box to file
    box->exportStructure("example.root", true);
}
