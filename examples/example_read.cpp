#include "SmartFactory.h"

#include <TFile.h>

#include <cstdlib>

#include <getopt.h>

#undef NDEBUG
#include <assert.h>


int main(int argc, char** argv)
{
    int verbose_flag = 0;
    int c = 0;

    while (1)
    {
      static struct option long_options[] =
        {
          /* These options set a flag. */
          {"verbose", no_argument,       &verbose_flag, 1},
          {"brief",   no_argument,       &verbose_flag, 0},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          printf ("option %s", long_options[option_index].name);
          if (optarg)
            printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case '?':
          /* getopt_long already printed an error message. */
          break;

        default:
          abort();
        }
    }

	// create factory
	SmartFactory * fac = new SmartFactory("factory1");

	// import from file and register in the factory
	// data will be stored in memory, file remains open
	TFile * f = fac->importStructure("example.root", verbose_flag);

	// list of registered objects 
	fac->listRegisteredObjects();

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
