/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Rafał Lalik <rafal.lalik@ph.tum.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <iostream>
#include <iomanip>
#include <TObject.h>
#include <TDirectory.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TKey.h>
#include <TCollection.h>
#include <TF1.h>

#include "SmartFactory.h"

float Normalize(TH1 * h, TH1 * href, bool extended = false)
{
	if (!extended)
	{
		Float_t integral_ref = href->Integral();
		Float_t integral_cur = h->Integral();

		h->Scale(integral_ref/integral_cur);
		return integral_ref/integral_cur;
	}
	else
	{
		TH1 * hc_mask = (TH1*)h->Clone("___XXX___hc_mask");
		TH1 * hr_mask = (TH1*)href->Clone("___XXX___hr_mask");

		hc_mask->Divide(h);
		hr_mask->Divide(href);

		TH1 * hc_temp = (TH1*)h->Clone("___XXX___hc_temp");
		TH1 * hr_temp = (TH1*)href->Clone("___XXX___hr_temp");

		hc_temp->Multiply(hr_mask);
		hr_temp->Multiply(hc_mask);

		float scale = Normalize(hc_temp, hr_temp);
		h->Scale( scale, 0 );

		hc_mask->Delete();
		hr_mask->Delete();
		hc_temp->Delete();
		hr_temp->Delete();

		return scale;
	}
	return 0.;
}

SmartFactory::SmartFactory(const std::string& name) :
factory_name(name), dirname(name), source(nullptr)
{
}

SmartFactory::SmartFactory(const std::string& name, const std::string& dir) :
factory_name(name), dirname(dir), source(nullptr)
{
}

SmartFactory::~SmartFactory()
{
	if (source) source->Close();
	if (target) target->Close();
}

void SmartFactory::validate()
{
	for (size_t i = 0; i < regnames.size(); ++i)
	{
		if (!regobjs[i])
		{
			std::cerr << "No objetc at " << regobjs[i] << std::endl;
			regobjs.erase(regobjs.begin()+i);
			--i;
		}
	}
}

void SmartFactory::listRegisterdObjects() const
{
	std::cout << "List of registered objects in " << factory_name << std::endl;
	std::cout << " Number of objects: " << regobjs.size() << std::endl;
	for (size_t i = 0; i < regnames.size(); ++i)
	{
// 		PR(regobjs[i]);
		std::cout	<< "* "
		<< regobjs[i]->GetName()
		<< " [ " << rawnames[i] << " ] "
		<< " [ " << regnames[i] << " ] "
		<< " : " << regobjs[i]->ClassName()
		<< " at " << regobjs[i]
		<< std::endl;
	}
}

bool SmartFactory::write(TFile* f, bool verbose)
{
	if (!f->IsOpen())
	{
		std::cerr << "File " << f->GetName() << " could not be open!" << std::endl;
		return false;
	}

	validate();

	f->cd();

	for (size_t i = 0; i < regobjs.size(); ++i)
	{
		std::string hname;
		std::string dir;
		splitDir(regnames[i], hname, dir);
		cdDir(f, dir.c_str());

		if (verbose)
			std::cout << std::left << std::setw(70) << std::string(" + Writing " + hname + " ... ");

		int res = regobjs[i]->Write(0, TObject::kOverwrite);

		f->cd();

		if (!res)
		{
			std::cerr << "failed writing " << hname << " to file! Aborting." << std::endl;
			std::exit(EXIT_FAILURE);
		}
		if (verbose)
			std::cout << " [ done ]" << std::endl;
	}

	return true;
}

bool SmartFactory::write(const char * filename, bool verbose)
{
	TFile * f = new TFile(filename, "RECREATE");

	bool res = write(f, verbose);

	f->Close();
	return res;
}


bool SmartFactory::exportStructure(TFile* f, bool verbose)
{
	if (!f->IsOpen())
	{
		std::cerr << "File " << f->GetName() << " could not be open!" << std::endl;
		return false;
	}

	validate();

	SmartFactoryObj obj;
	obj.objnames = regnames;

	f->cd();

	obj.Write(this->factory_name.c_str());

	return write(f, verbose);
}

bool SmartFactory::exportStructure(const char * filename, bool verbose)
{
	TFile * f = new TFile(filename, "RECREATE");

	bool res = exportStructure(f, verbose);

	f->Close();
	return res;
}

bool SmartFactory::importStructure(TFile* f, bool verbose)
{
	if (!f->IsOpen())
	{
		std::cerr << "File " << f->GetName() << " could not be open!" << std::endl;
		return false;
	}

	setSource(f);

	SmartFactoryObj * obj;
	f->GetObject<SmartFactoryObj>(this->factory_name.c_str(), obj);

	for (uint i = 0; i < obj->objnames.size(); ++i)
	{
		f->cd();
		if (verbose)
			std::cout << "Importing " << obj->objnames[i];

		TObject * o = this->getObject(f, obj->objnames[i].c_str());
		if (o)
		{
			this->RegObject(obj->objnames[i].c_str());
			if (verbose)
				std::cout << " [ done ] " << std::endl;
		}
		else
		{
			if (verbose)
				std::cout << " [ failed ] " << std::endl;
		}
	}

	return true;
}

TFile * SmartFactory::importStructure(const char * filename, bool verbose)
{
	source = new TFile(filename, "READ");

	bool res = importStructure(source, verbose);

	if (!res)
	{
		source->Close();
		source = nullptr;
	}

	return source;
}

TObject* SmartFactory::getObject(const std::string& name, const std::string & dir) const
{
	std::vector<std::string>::const_iterator it = regnames.begin();
	if (std::find(regnames.begin(), regnames.end(), name) != regnames.end())
		return regobjs[int(it-regnames.begin())];

	if (!source)
		return nullptr;

	std::string fn = format(name);
	TObject * obj = nullptr;
	if (dir.size())
		obj = getObject(source, fn, dir);
	else
		obj = getObject(source, fn);

	if (obj)
		return obj;

	return nullptr;
}

TObject* SmartFactory::getObject(TDirectory * srcdir, const std::string& fullname)
{
	std::string hname;
	std::string dir;
	splitDir(fullname, hname, dir);
	return getObject(srcdir, hname, dir);
}

TObject* SmartFactory::getObject(TDirectory * srcdir, const std::string& name, const std::string& dir)
{
	static pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&mutex1);
	TObject  * ptr = nullptr;

	if (srcdir)
	{
		if (dir.size())
			srcdir->cd(dir.c_str());
		else
			srcdir->cd();

		TDirectory * d = gDirectory;

		d->GetObject(name.c_str(), ptr);

		srcdir->cd();
	}

	pthread_mutex_unlock(&mutex1);
	return ptr;
}

void SmartFactory::splitDir(const std::string& fullname, std::string& name, std::string& dir)
{
	size_t slash_pos = fullname.find_last_of("/");
	if (slash_pos != std::string::npos)
	{
		dir = fullname.substr(0, slash_pos);
		name = fullname.substr(slash_pos+1, std::string::npos);
	}
	else
	{
		name = fullname;
		dir = "";
	}
}

bool SmartFactory::cdDir(TFile * target, const char * dir, bool automkdir) const
{
	if (!target)
		return false;

	if (!target->GetDirectory(dir))
		if (automkdir)
			if (target->mkdir(dir, dir))
				return target->cd(dir);

	return false;
}

TCanvas * SmartFactory::RegCanvas(const char* name, const char* title, int width, int height)
{
	std::string fulltitle = format(title);
	std::string fullname = format(name);
	std::string hname;
	std::string dir;
	splitDir(fullname, hname, dir);

	// try to get object from file
	TCanvas * c = nullptr;//(TCanvas*)getObject(hname, dir); // FIXME disable fetching canvases from file
	if (c)
	{
		std::string cname = c->GetName();
		// 		c = (TCanvas *)c->DrawClone(c->GetName());
		// 		c->SetName(cname.c_str());
		// 		PR("clone");
		c->Draw();

		// 		TList * l = c->GetListOfPrimitives();
		// 		for (size_t i = 0; i < l->GetEntries(); ++i) {
		// 			c->cd(1+i);
		// 			gPad->Draw();
		// 		}
	}
	else
	{
		c = new TCanvas(hname.c_str(), fulltitle.c_str(), width, height);
		c->SetCanvasSize(width, height);
		// 		PR("create");PR(hname.c_str());
	}

	if (c) {
		rawnames.push_back(name);
		regnames.push_back(fullname);
		regobjs.push_back(c);
	}

	return c;
}

TCanvas* SmartFactory::RegCanvas(const char* name, const char* title, int width, int height, int divsqr)
{
	TCanvas * c = RegCanvas(name, title, width, height);
	// 	if (!this->source)
	if (!c->GetListOfPrimitives()->GetEntries())
		c->DivideSquare(divsqr);
	return c;
}

TCanvas* SmartFactory::RegCanvas(const char* name, const char* title, int width, int height, int cols, int rows)
{
	TCanvas * c = RegCanvas(name, title, width, height);
	// 	if (!this->source)
	if (!c->GetListOfPrimitives()->GetEntries())
		c->Divide(cols, rows);
	return c;
}

TObject* SmartFactory::RegClone(TObject* obj, const std::string& new_name)
{
	std::string fullname = format(new_name);
	std::string hname;
	std::string dir;
	splitDir(fullname, hname, dir);

	TObject * c = obj->Clone(hname.c_str());

	if (c)
	{
		rawnames.push_back(new_name);
		regnames.push_back(fullname);
		regobjs.push_back(c);
	}

	return c;
}

TObject* SmartFactory::RegObject(const std::string& name)
{
	std::string fullname = format(name);
	std::string hname;
	std::string dir;
	splitDir(fullname, hname, dir);

	// try to get object from file
	TObject* obj = getObject(hname, dir);

	if (obj) {
		rawnames.push_back(name);
		regnames.push_back(fullname);
		regobjs.push_back(obj);
	}

	return obj;
}

TObject* SmartFactory::RegObject(TObject* obj)
{
	if (obj)
	{
		rawnames.push_back(obj->GetName());
		regnames.push_back(obj->GetName());
		regobjs.push_back(obj);
	}

	return obj;
}

std::string SmartFactory::format(const std::string & name) const
{
	std::string n1 = placeholder(name, '#', dirname);
	std::string n2 = placeholder(n1, '@', factory_name);
	return n2;
}

std::string SmartFactory::placeholder(const std::string& pattern, char c, const std::string& value) const
{
	std::string n = pattern;

	// replace 'c' with 'value'

	size_t p = std::string::npos;
	while ( (p = n.find_first_of(c)) != std::string::npos)
	{
		n.replace(p, 1, value);
	}
	return n;
}

void SmartFactory::rename(const std::string & newname)
{
	factory_name = newname;
	renameAllObjects();
}

void SmartFactory::chdir(const std::string& newdir)
{
	dirname = newdir;
	renameAllObjects();
}

void SmartFactory::renameAllObjects()
{
	for (size_t i = 0; i < regobjs.size(); ++i)
	{
		regnames[i] = format(rawnames[i]);
		std::string hname;
		std::string dir;
		splitDir(regnames[i], hname, dir);

		if (regobjs[i]->InheritsFrom("TPad"))
			((TPad*)regobjs[i])->SetName(hname.c_str());
		else
			if (regobjs[i]->InheritsFrom("TNamed"))
				((TNamed*)regobjs[i])->SetName(hname.c_str());
			else {
				std::cerr << "Unknow class for SetName "
				<< regobjs[i]->ClassName()
				<< std::endl;
				std::exit(EXIT_FAILURE);
			}
	}
}

SmartFactory & SmartFactory::operator+=(const SmartFactory& fa)
{
	for (size_t i = 0; i < regobjs.size(); ++i)
	{
		if (regobjs[i]->InheritsFrom("TH1"))
		{
			Bool_t res = ((TH1*)regobjs[i])->Add((TH1*)fa.regobjs[i]);
			if (!res)
			{
				std::cerr << "Failed adding histogram " << regobjs[i]->GetName() << std::endl;
			}
		}
	}
	return *this;
}

SmartFactory & SmartFactory::operator-=(const SmartFactory& fa)
{
	for (size_t i = 0; i < regobjs.size(); ++i)
	{
		if (regobjs[i]->InheritsFrom("TH1"))
		{
			Bool_t res = ((TH1*)regobjs[i])->Add((TH1*)fa.regobjs[i], -1);
			if (!res)
			{
				std::cerr << "Failed subtracting histogram " << regobjs[i]->GetName() << std::endl;
			}
		}
	}
	return *this;
}

SmartFactory & SmartFactory::operator*=(const SmartFactory& fa)
{
	for (size_t i = 0; i < regobjs.size(); ++i)
	{
		if (regobjs[i]->InheritsFrom("TH1"))
		{
			Bool_t res = ((TH1*)regobjs[i])->Multiply((TH1*)fa.regobjs[i]);
			if (!res)
			{
				std::cerr << "Failed multiplying histogram " << regobjs[i]->GetName() << std::endl;
			}

		}
	}
	return *this;
}

SmartFactory & SmartFactory::operator/=(const SmartFactory& fa)
{
	for (size_t i = 0; i < regobjs.size(); ++i)
	{
		if (regobjs[i]->InheritsFrom("TH1"))
		{
			Bool_t res = ((TH1*)regobjs[i])->Divide((TH1*)fa.regobjs[i]);
			if (!res)
			{
				std::cerr << "Failed dividing histogram " << regobjs[i]->GetName() << std::endl;
			}

		}
	}
	return *this;
}

SmartFactory & SmartFactory::operator*=(Float_t num)
{
	for (size_t i = 0; i < regobjs.size(); ++i)
	{
		if (regobjs[i]->InheritsFrom("TH1"))
		{
			((TH1*)regobjs[i])->Scale(num);
		}
	}
	return *this;
}

SmartFactory & SmartFactory::operator/=(Float_t num)
{
	for (size_t i = 0; i < regobjs.size(); ++i)
	{
		if (regobjs[i]->InheritsFrom("TH1"))
		{
			((TH1*)regobjs[i])->Scale(1.0/num);
		}
	}
	return *this;
}

void SmartFactory::norm(const SmartFactory& fa, bool extended)
{
	for (size_t i = 0; i < regobjs.size(); ++i)
	{
		if (regobjs[i]->InheritsFrom("TH1"))
		{
			Normalize( ((TH1*)regobjs[i]), ((TH1*)fa.regobjs[i]), extended);
		}
	}
}

void SmartFactory::norm(Float_t num)
{
	for (size_t i = 0; i < regobjs.size(); ++i)
	{
		if (regobjs[i]->InheritsFrom("TH1"))
		{
			Float_t integral_ref = num;
			Float_t integral_cur = ((TH1*)regobjs[i])->Integral();

			((TH1*)regobjs[i])->Scale(integral_ref/integral_cur);
		}
	}
}

void SmartFactory::setTitleForAll(const TString & title)
{
	for (size_t i = 0; i < regobjs.size(); ++i)
	{
		if (regobjs[i]->InheritsFrom("TH1"))
		{
			((TH1*)regobjs[i])->SetTitle(title);
		}
	}
}

void SmartFactory::print_counts() const
{
	for (size_t i = 0; i < regobjs.size(); ++i)
	{
		if (regobjs[i]->InheritsFrom("TH1"))
		{
			printf(" %s %f\n", ((TH1*)regobjs[i])->GetName(), ((TH1*)regobjs[i])->GetEntries());
		}
	}
}

void SmartFactory::print_integrals() const
{
	for (size_t i = 0; i < regobjs.size(); ++i)
	{
		if (regobjs[i]->InheritsFrom("TH1"))
		{
			printf(" %s %f\n", ((TH1*)regobjs[i])->GetName(), ((TH1*)regobjs[i])->Integral());
		}
	}
}