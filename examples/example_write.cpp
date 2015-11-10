#include "SmartFactory.h"

int main()
{
	// create factory
	
	SmartFactory * fac = new SmartFactory("factory1");

	// fill with histograms
	char hname[100];
	for (int i = 0; i < 10; ++i)
	{
		sprintf(hname, "hist_%05d", i);
		TH1F * h = fac->RegTH1<TH1F>(hname, "Histogram - loop", 100, -5, 5);
		h->FillRandom("gaus", 100000);
	}

	TH1F * hist1 = fac->RegTH1<TH1F>("dir1/hist1", "Histogram 1", 100, -5, 5);
	TH1F * hist2 = fac->RegTH1<TH1F>("hist2", "Histogram 2", 100, -5, 5);
	TH1F * hist3 = fac->RegTH1<TH1F>("dir1/dir2/hist3", "Histogram 3", 100, -5, 5);
	TH1F * hist4 = fac->RegTH1<TH1F>("@@@d/hist_@@@a_placeholders", "Histogram 4", 100, -5, 5);

	fac->rename("renamed_factory");
	fac->chdir("renamed_directory");

	hist1->FillRandom("gaus", 100000);
	hist2->FillRandom("gaus", 100000);
	hist3->FillRandom("gaus", 100000);

// 	TCanvas * can = fac->RegCanvas("can1", "Canvas", 800, 800, 4);

	// list objects
	fac->listRegisteredObjects();

	// export factory to file
	fac->exportStructure("example.root", true);
}