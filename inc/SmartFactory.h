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
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
// ROOT
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TNamed.h>
#include <TObjArray.h>
#include <TObjString.h>

#define PR(x)                                                                  \
    std::cout << "++DEBUG: " << #x << " = |" << x << "| (" << __FILE__ << ", " \
              << __LINE__ << ")\n";

class TDirectory;
class TCanvas;
class TFile;

struct SmartFactoryObj : public TNamed
{
    std::vector<std::string> objnames;
    TObjString objects_name;
    TObjString directory_name;

    ClassDef(SmartFactoryObj, 1);
};

class SmartFactory
{
  public:
    SmartFactory(const char* name);
    SmartFactory(const char* name, const char* dir);
    SmartFactory(const SmartFactory& fac);
    virtual ~SmartFactory();

    // operators
    SmartFactory& operator=(const SmartFactory& fa);
    SmartFactory& operator+=(const SmartFactory& fa);
    SmartFactory& operator-=(const SmartFactory& fa);
    SmartFactory& operator*=(const SmartFactory& fa);
    SmartFactory& operator/=(const SmartFactory& fa);

    SmartFactory& operator*=(Float_t num);
    SmartFactory& operator/=(Float_t num);

    virtual std::string name() const { return fac_name; }
    virtual std::string objects_name() const { return obj_name; }
    virtual std::string directory_name() const { return dir_name; }

    // objects creation
    // histograms
    template <class T>
    T* RegTH1(const char* name, const char* title, int bins, double min,
              double max, bool sumw2 = true);
    template <class T>
    T* RegTH1(const char* name, const char* title, int bins, double* arr,
              bool sumw2 = true);

    template <class T>
    T* RegTH2(const char* name, const char* title, int xbins, double xmin,
              double xmax, int ybins, double ymin, double ymax,
              bool sumw2 = true);
    template <class T>
    T* RegTH2(const char* name, const char* title, int xbins, double* xarr,
              int ybins, double* yarr, bool sumw2 = true);

    template <class T>
    T* RegTH3(const char* name, const char* title, int xbins, double xmin,
              double xmax, int ybins, double ymin, double ymax, int zbins,
              double zmin, double zmax, bool sumw2 = true);
    template <class T>
    T* RegTH3(const char* name, const char* title, int xbins, double* xarr,
              int ybins, double* yarr, int zbins, double* zarr,
              bool sumw2 = true);

    template <class T> T* RegGraph(const char* name, int points);

    // canvases
    TCanvas* RegCanvas(const char* name, const char* title, int width,
                       int height);
    TCanvas* RegCanvas(const char* name, const char* title, int width,
                       int height, int divsqr);
    TCanvas* RegCanvas(const char* name, const char* title, int width,
                       int height, int cols, int rows);

    // TObject general
    TObject* RegObject(TObject* obj);
    TObject* RegObject(const std::string& name);

    // register clone of the object
    TObject* RegClone(TObject* obj, const std::string& new_name);

    // file targets
    virtual void setSource(TFile* file) { source = file; }
    virtual TFile* getSource() const { return source; }
    virtual void setTarget(TFile* file) { target = file; }
    virtual TFile* getTarget() const { return target; }

    virtual void setSourceName(const char* filename) { source_name = filename; }
    virtual const char* getSourceName() const { return source_name.c_str(); }
    virtual void setTargetName(const char* filename) { target_name = filename; }
    virtual const char* getTargetName() const { return target_name.c_str(); }

    // validate list of objects
    virtual void validate();

    // print list of objects
    virtual void listRegisteredObjects() const;

    virtual TObject* getObject(const std::string& name,
                               const std::string& dir = "") const;
    static TObject* getObject(TDirectory* srcdir, const std::string& fullname);
    static TObject* getObject(TDirectory* srcdir, const std::string& name,
                              const std::string& dir);

    virtual bool write(TFile* f /* = nullptr*/, bool verbose = false);
    virtual bool write(const char* filename /* = nullptr*/,
                       bool verbose = false);

    virtual bool exportStructure(TFile* target, bool verbose = false);
    virtual bool exportStructure(const char* filename, bool verbose = false);

    virtual bool importStructure(TFile* target, bool verbose = false);
    virtual TFile* importStructure(const char* filename, bool verbose = false);

    virtual void set_name(const char* name) { fac_name = name; }
    virtual void rename(const char* newname);
    virtual void chdir(const char* newdir);
    virtual void reset();

    // operation on histograms
    virtual void norm(const SmartFactory& fa, bool extended = false);
    virtual void norm(Float_t num);

    virtual void printCounts() const;
    virtual void printIntegrals() const;

    // global modifiers
    virtual void setTitleForAll(const TString& title);
    virtual std::string format(const std::string& name) const;

    static std::string placeholder(const std::string& pattern,
                                   const std::string& str,
                                   const std::string& value);
    static std::string placeholder(const std::string& pattern, char c,
                                   const std::string& value);

    virtual void callFunctionOnObjects(const SmartFactory* fac,
                                       void (*fun)(TObject* dst, const TObject* src));

    virtual int findIndex(TObject* obj) const;
    virtual TObject* findObject(int index) const;
    virtual int findIndexByRawname(const std::string& name) const;

    virtual void setFileOwner(bool owner = true) { own_file = owner; }

  private:
    void operator+(const SmartFactory&){};
    void operator-(const SmartFactory&){};

    void renameAllObjects();

    static void splitDir(const std::string& fullname, std::string& name,
                         std::string& dir);
    bool cdDir(TFile* target, const char* dir, bool automkdir = true) const;

  public:
    struct ObjectData
    {
        std::string raw_name;
        std::string fmtnames;
        std::string reg_name;
        TObject* object;
    };

  private:
    std::string fac_name;
    std::string obj_name;
    std::string dir_name;

    std::vector<ObjectData> regobjects;

    // file targets
    TFile* source;
    TFile* target;
    Bool_t shared;

    std::string source_name;
    std::string target_name;

    bool own_file;
};

template <class T>
T* SmartFactory::RegTH1(const char* name, const char* title, int bins,
                        double min, double max, bool sumw2)
{
    std::string fullname = format(name);
    std::string fulltitle = format(title);

    std::string hname;
    std::string dir;
    splitDir(fullname, hname, dir);

    // try to get object from file
    T* h = (T*)getObject(hname, dir);
    if (!h)
    {
        h = new T(hname.c_str(), fulltitle.c_str(), bins, min, max);
        if (sumw2 and 0 == h->GetSumw2N()) h->Sumw2();
    }
    if (h)
    {
        ObjectData od;
        od.raw_name = name;
        od.reg_name = fullname;
        od.object = h;
        regobjects.push_back(od);
    }
    return h;
}

template <class T>
T* SmartFactory::RegTH1(const char* name, const char* title, int bins,
                        double* arr, bool sumw2)
{
    std::string fullname = format(name);
    std::string fulltitle = format(title);

    std::string hname;
    std::string dir;
    splitDir(fullname, hname, dir);

    // try to get object from file
    T* h = (T*)getObject(hname, dir);
    if (!h)
    {
        h = new T(hname.c_str(), fulltitle.c_str(), bins, arr);
        if (sumw2 and 0 == h->GetSumw2N()) h->Sumw2();
    }
    if (h)
    {
        ObjectData od;
        od.raw_name = name;
        od.reg_name = fullname;
        od.object = h;
        regobjects.push_back(od);
    }
    return h;
}

template <class T>
T* SmartFactory::RegTH2(const char* name, const char* title, int xbins,
                        double xmin, double xmax, int ybins, double ymin,
                        double ymax, bool sumw2)
{
    std::string fullname = format(name);
    std::string hname;
    std::string dir;
    splitDir(fullname, hname, dir);

    // try to get object from file
    T* h = (T*)getObject(hname, dir);
    if (!h)
    {
        h = new T(hname.c_str(), title, xbins, xmin, xmax, ybins, ymin, ymax);
        if (sumw2 and 0 == h->GetSumw2N()) h->Sumw2();
    }
    if (h)
    {
        ObjectData od;
        od.raw_name = name;
        od.reg_name = fullname;
        od.object = h;
        regobjects.push_back(od);
    }
    return h;
}

template <class T>
T* SmartFactory::RegTH2(const char* name, const char* title, int xbins,
                        double* xarr, int ybins, double* yarr, bool sumw2)
{
    std::string fullname = format(name);
    std::string hname;
    std::string dir;
    splitDir(fullname, hname, dir);

    // try to get object from file
    T* h = (T*)getObject(hname, dir);
    if (!h)
    {
        h = new T(hname.c_str(), title, xbins, xarr, ybins, yarr);
        if (sumw2 and 0 == h->GetSumw2N()) h->Sumw2();
    }
    if (h)
    {
        ObjectData od;
        od.raw_name = name;
        od.reg_name = fullname;
        od.object = h;
        regobjects.push_back(od);
    }
    return h;
}

template <class T>
T* SmartFactory::RegTH3(const char* name, const char* title, int xbins,
                        double xmin, double xmax, int ybins, double ymin,
                        double ymax, int zbins, double zmin, double zmax,
                        bool sumw2)
{
    std::string fullname = format(name);
    std::string hname;
    std::string dir;
    splitDir(fullname, hname, dir);

    // try to get object from file
    T* h = (T*)getObject(hname, dir);
    if (!h)
    {
        h = new T(hname.c_str(), title, xbins, xmin, xmax, ybins, ymin, ymax,
                  zbins, zmin, zmax);
        if (sumw2 and 0 == h->GetSumw2N()) h->Sumw2();
    }
    if (h)
    {
        ObjectData od;
        od.raw_name = name;
        od.reg_name = fullname;
        od.object = h;
        regobjects.push_back(od);
    }
    return h;
}

template <class T>
T* SmartFactory::RegTH3(const char* name, const char* title, int xbins,
                        double* xarr, int ybins, double* yarr, int zbins,
                        double* zarr, bool sumw2)
{
    std::string fullname = format(name);
    std::string hname;
    std::string dir;
    splitDir(fullname, hname, dir);

    // try to get object from file
    T* h = (T*)getObject(hname, dir);
    if (!h)
    {
        h = new T(hname.c_str(), title, xbins, xarr, ybins, yarr, zbins, zarr);
        if (sumw2 and 0 == h->GetSumw2N()) h->Sumw2();
    }
    if (h)
    {
        ObjectData od;
        od.raw_name = name;
        od.reg_name = fullname;
        od.object = h;
        regobjects.push_back(od);
    }
    return h;
}

template <class T> T* SmartFactory::RegGraph(const char* name, int points)
{
    std::string fullname = format(name);

    std::string hname;
    std::string dir;
    splitDir(fullname, hname, dir);

    // try to get object from file
    T* h = (T*)getObject(hname, dir);
    if (!h)
    {
        h = new T(points);
        h->SetName(hname.c_str());
    }
    if (h)
    {
        ObjectData od;
        od.raw_name = name;
        od.reg_name = fullname;
        od.object = h;
        regobjects.push_back(od);
    }
    return h;
}

#endif // SMARTFACTORY_H
