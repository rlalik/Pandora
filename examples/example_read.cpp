#include <cstdlib>

#undef NDEBUG
#include <assert.h>

#include "SmartFactory.h"

#include "TFile.h"

int main()
{
	// create factory
	SmartFactory * fac = new SmartFactory("factory1");

	// import from file and register in the factory
	// data will be stored in memory, file remains open
	TFile * f = fac->importStructure("example.root");

	// list of registered objects 
	fac->listRegisterdObjects();

	// you can fetch specific object by its name
	TH1F * h1 = (TH1F*)fac->getObject("dir1/hist1");
	TH1F * h2 = (TH1F*)fac->getObject("hist2");
	TH1F * h3 = (TH1F*)fac->getObject("hist3", "dir1/dir2");
	TH1F * h4 = (TH1F*)fac->getObject("@@@d/hist_@@@a_placeholders");

	// if failed, then objects are not read from file
	assert(h1 != nullptr);
	assert(h2 != nullptr);
	assert(h3 != nullptr);

	// file must be closed by user
	f->Close();
}
