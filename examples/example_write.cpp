#include "../SmartFactory.h"

int main()
{
	// create factory
	
	SmartFactory * fac = new SmartFactory("factory1");

	char hname[100];
	for (int i = 0; i < 10; ++i)
	{
		sprintf(hname, "hist_%05d", i);
		TH1F * h = fac->RegTH1<TH1F>(hname, "Histogram - loop", 10, -5, 5);
		h->FillRandom("gaus", 1000);
	}

	TH1F * hist1 = fac->RegTH1<TH1F>("dir1/hist1", "Histogram 1", 10, -5, 5);
	TH1F * hist2 = fac->RegTH1<TH1F>("hist2", "Histogram 2", 10, -5, 5);
	TH1F * hist3 = fac->RegTH1<TH1F>("dir1/dir2/hist3", "Histogram 3", 10, -5, 5);

	hist1->FillRandom("gaus", 1000);
	hist2->FillRandom("gaus", 1000);
	hist3->FillRandom("gaus", 1000);

// 	TCanvas * can = fac->RegCanvas("can1", "Canvas", 800, 800, 4);

	fac->listRegisterdObjects();
	fac->exportStructure("example.root", false);
}