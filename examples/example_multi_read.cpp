#include <cstdlib>

#undef NDEBUG
#include <assert.h>

#include <pandora.hpp>

#include <TCanvas.h>
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>

const int bar_limit = 50;
const char hist_pattern[] = "hist_%06d";
const char can_pattern[] = "can_%06d";

void bar(int i)
{
    printf(".");
    if (i and (i + 1) % bar_limit == 0) printf("  %8d\n", i + 1);
}

void write_func()
{
    // create box

    pandora::pandora* box = new pandora::pandora("box1");

    // fill with histograms
    char hname[100];
    for (int i = 0; i < 100; ++i)
    {
        sprintf(hname, hist_pattern, i);
        TH2F* h = box->reg_hist<TH2F>(hname, "Histogram - loop", 100, -5, 5, 100, -5, 5);

        for (int j = 0; j < 100 * 100; ++j)
            h->SetBinContent(j + 1, sqrt(j));

        // 		sprintf(hname, can_pattern, i);
        // 		TCanvas * c = box->RegCanvas(hname, "Canvas - loop", 800, 600);
        // 		c->cd(0);
        // 		h->Draw("colz");

        bar(i);
    }

    box->add_placeholder("{dir}", "renamed_box");
    box->add_placeholder("{dir2}", "subdir");
    box->add_placeholder("{analysis}", "renamed_directory");

    // list objects
    // 	box->listRegisterdObjects();

    // export box to file
    box->import_structure("example_multi.root", true);
}

void loop_read_func()
{
    // create box
    pandora::pandora* box = new pandora::pandora("box1");

    // import from file and register in the box
    // data will be stored in memory, file remains open
    TFile* f = box->import_structure("example_multi.root");

    // list of registered objects
    // 	box->listRegisterdObjects();

    // you can fetch specific object by its name
    char hname[100];
    for (int i = 0; i < 100; ++i)
    {
        sprintf(hname, hist_pattern, i);
        TH2F* h1 = (TH2F*)box->get_object(hname);
        // if failed, then objects are not read from file
        assert(h1 != nullptr);

        // 		sprintf(hname, can_pattern, i);
        // 		TCanvas * c1 = (TCanvas*)box->getObject(hname);
        // 		assert(c1 != nullptr);
    }

    delete box;

    // file must be closed by user
    f->Close();
}

int main(int argc, char** argv)
{
    int loops = 100;

    if (argc > 1) loops = atoi(argv[1]);

    printf("Create root file\n");
    write_func();

    printf("Read root file in %d loops\n", loops);
    for (int i = 0; i < loops; ++i)
    {
        loop_read_func();

        bar(i);
    }
}
