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

#ifndef SMARTFACTORY_H
#define SMARTFACTORY_H

// std
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
// ROOT
#include <TH1.h>
#include <TH2.h>
#include <TNamed.h>
#include <TObjArray.h>


#define PR(x) std::cout << "++DEBUG: " << #x << " = |" << x << "| (" << __FILE__ << ", " << __LINE__ << ")\n";

class TDirectory;
class TCanvas;
class TFile;

struct SmartFactoryObj : public TNamed
{
	std::vector<std::string> objnames;
	ClassDef(SmartFactoryObj, 1);
};

class SmartFactory
{
public:
	SmartFactory(const std::string & name);
	SmartFactory(const std::string & name, const std::string & dir);
	~SmartFactory();

	inline std::string name() const { return factory_name; }

	// objects creation
	// histograms
	template<class T>
	T* RegTH1(const char* name, const char* title, int bins, float min, float max, bool sumw2 = true);

	template<class T>
	T* RegTH2(const char* name, const char* title,
			  int xbins, float xmin, float xmax, int ybins, float ymin, float ymax, bool sumw2 = true);

	template<class T>
	T* RegGraph(const char* name, int points);

	// canvases
	TCanvas * RegCanvas(const char* name, const char* title, int width, int height);
	TCanvas * RegCanvas(const char* name, const char* title, int width, int height, int divsqr);
	TCanvas * RegCanvas(const char* name, const char* title, int width, int height, int cols, int rows);

	// TObject general
	TObject * RegObject(TObject * obj);
	TObject * RegObject(const std::string & name);

	// register clone of the object
	TObject * RegClone(TObject * obj, const std::string & new_name);

	// file targets
	inline void setSource(TFile * file) { source = file; }
	inline void setTarget(TFile * file) { target = file; }

	inline void setSourceName(const char * filename) { source_name = filename; }
	inline void setTargetName(const char * filename) { target_name = filename; }

	// validate list of objects
	void validate();

	// print list of objects
	void listRegisterdObjects() const;

	TObject * getObject(const std::string & name, const std::string & dir = "") const;
	static TObject * getObject(TDirectory * srcdir, const std::string & fullname);
	static TObject * getObject(TDirectory * srcdir, const std::string & name, const std::string & dir);

	bool write(TFile * f/* = nullptr*/, bool verbose = false);
	bool write(const char * filename/* = nullptr*/, bool verbose = false);

	bool exportStructure(TFile * target, bool verbose = false);
	bool exportStructure(const char * filename, bool verbose = false);

	bool importStructure(TFile * target, bool verbose = false);
	TFile * importStructure(const char * filename, bool verbose = false);

	void rename(const std::string & newname);
	void chdir(const std::string & newdir);

	// operators
	SmartFactory & operator+=(const SmartFactory & fa);
	SmartFactory & operator-=(const SmartFactory & fa);
	SmartFactory & operator*=(const SmartFactory & fa);
	SmartFactory & operator/=(const SmartFactory & fa);

	SmartFactory & operator*=(Float_t num);
	SmartFactory & operator/=(Float_t num);

	// operation on histograms
	void norm(const SmartFactory & fa, bool extended = false);
	void norm(Float_t num);

	void print_counts() const;
	void print_integrals() const;

	// global modifiers
	void setTitleForAll(const TString & title);
	std::string format(const std::string & name) const;

private:
	void operator+(const SmartFactory &) {};
	void operator-(const SmartFactory &) {};

	void renameAllObjects();

	std::string placeholder(const std::string & pattern, char c, const std::string & value) const;

	static void splitDir(const std::string & fullname, std::string & name, std::string & dir);
	bool cdDir(TFile * target, const char * dir, bool automkdir = true) const;

private:
	std::string factory_name;
	std::string dirname;
	std::vector<std::string> rawnames;
	std::vector<std::string> fmtnames;
	std::vector<std::string> regnames;
	std::vector<TObject*> regobjs;

	// file targets
	TFile * source;
	TFile * target;

	std::string source_name;
	std::string target_name;
};


template<class T>
T* SmartFactory::RegTH1(const char* name, const char* title, int bins, float min, float max, bool sumw2)
{
	std::string fullname = format(name);
	std::string fulltitle = format(title);

	std::string hname;
	std::string dir;
	splitDir(fullname, hname, dir);
	
	// try to get object from file
	T * h = (T*)getObject(hname, dir);
	if (!h) {
		h = new T(hname.c_str(), fulltitle.c_str(), bins, min, max);
		if (sumw2) h->Sumw2();
	}
	if (h) {
		rawnames.push_back(name);
		regnames.push_back(fullname);
		regobjs.push_back(h);
	}
	return h;
}

template<class T>
T* SmartFactory::RegTH2(const char* name, const char* title,
		  int xbins, float xmin, float xmax, int ybins, float ymin, float ymax, bool sumw2)
{
	std::string fullname = format(name);
	std::string hname;
	std::string dir;
	splitDir(fullname, hname, dir);
	
	// try to get object from file
	T * h = (T*)getObject(hname, dir);
	if (!h) {
		h = new T(hname.c_str(), title, xbins, xmin, xmax, ybins, ymin, ymax);
		if (sumw2) h->Sumw2();
	}
	if (h) {
		rawnames.push_back(name);
		regnames.push_back(fullname);
		regobjs.push_back(h);
	}
	return h;
}

template<class T>
T* SmartFactory::RegGraph(const char* name, int points)
{
	std::string fullname = format(name);

	std::string hname;
	std::string dir;
	splitDir(fullname, hname, dir);
	
	// try to get object from file
	T * h = (T*)getObject(hname, dir);
	if (!h) {
		h = new T(points);
		h->SetName(hname.c_str());
	}
	if (h) {
		rawnames.push_back(name);
		regnames.push_back(fullname);
		regobjs.push_back(h);
	}
	return h;
}

#endif // SMARTFACTORY_H
