#include "../SmartFactory.h"

#include "TFile.h"

int main()
{
	// create factory
	
	SmartFactory * fac = new SmartFactory("factory1");

	fac->importStructure("example.root");

	fac->listRegisterdObjects();

// 	gDirectory->ls();
}
