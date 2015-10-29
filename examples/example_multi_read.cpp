#include <cstdlib>

#undef NDEBUG
#include <assert.h>

#include "SmartFactory.h"

#include "TFile.h"

const int bar_limit = 50;
void write_func()
{
	// create factory
	
	SmartFactory * fac = new SmartFactory("factory1");

	// fill with histograms
	char hname[100];
	for (int i = 0; i < 1000; ++i)
	{
		sprintf(hname, "hist_%06d", i);
		TH1F * h = fac->RegTH1<TH1F>(hname, "Histogram - loop", 100, -5, 5);
		h->FillRandom("gaus", 100000);

		printf(".");
		if (i and (i+1) % bar_limit == 0)
			printf("  %8d\n", i+1);
	}

	fac->rename("renamed_factory");
	fac->chdir("renamed_directory");

	// list objects
// 	fac->listRegisterdObjects();

	// export factory to file
	fac->exportStructure("example_multi.root", true);
}

void loop_read_func()
{
	// create factory
	SmartFactory * fac = new SmartFactory("factory1");

	// import from file and register in the factory
	// data will be stored in memory, file remains open
	TFile * f = fac->importStructure("example_multi.root");

	// list of registered objects 
// 	fac->listRegisterdObjects();

	// you can fetch specific object by its name
	TH1F * h1 = (TH1F*)fac->getObject("hist_000000");
	// if failed, then objects are not read from file
	assert(h1 != nullptr);

	delete fac;

	// file must be closed by user
	f->Close();
}

int main(int argc, char ** argv)
{
	int loops = 100;

	if (argc > 1)
		loops = atoi(argv[1]);

	printf("Create root file\n");
	write_func();

	printf("Read root file in %d loops\n", loops);
	for (int i = 0; i < loops; ++i)
	{
		loop_read_func();
		printf(".");
		if (i and i % 50 == 0)
			printf("  %8d\n", i);
	}
}
