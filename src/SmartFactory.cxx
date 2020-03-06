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

#include <TCanvas.h>
#include <TCollection.h>
#include <TDirectory.h>
#include <TF1.h>
#include <TFile.h>
#include <TKey.h>
#include <TObject.h>
#include <iomanip>
#include <iostream>

#include "SmartFactory.h"

float Normalize(TH1* h, TH1* href, bool extended = false)
{
    if (!extended)
    {
        Float_t integral_ref = href->Integral();
        Float_t integral_cur = h->Integral();

        h->Scale(integral_ref / integral_cur);
        return integral_ref / integral_cur;
    }
    else
    {
        TH1* hc_mask = (TH1*)h->Clone("___XXX___hc_mask");
        TH1* hr_mask = (TH1*)href->Clone("___XXX___hr_mask");

        hc_mask->Divide(h);
        hr_mask->Divide(href);

        TH1* hc_temp = (TH1*)h->Clone("___XXX___hc_temp");
        TH1* hr_temp = (TH1*)href->Clone("___XXX___hr_temp");

        hc_temp->Multiply(hr_mask);
        hr_temp->Multiply(hc_mask);

        float scale = Normalize(hc_temp, hr_temp);
        h->Scale(scale, 0);

        hc_mask->Delete();
        hr_mask->Delete();
        hc_temp->Delete();
        hr_temp->Delete();

        return scale;
    }
    return 0.;
}

SmartFactory::SmartFactory(const char* name)
    : fac_name(name), obj_name(name), dir_name(name), source(nullptr), shared(kFALSE),
      own_file(false)
{
}

SmartFactory::SmartFactory(const char* name, const char* dir)
    : fac_name(name), obj_name(name), dir_name(dir), source(nullptr), shared(kFALSE),
      own_file(false)
{
}

SmartFactory::SmartFactory(const SmartFactory& fac)
{
    fac_name = fac.fac_name;
    obj_name = fac.obj_name;
    dir_name = fac.dir_name;

    source = fac.source;
    target = fac.target;
    source_name = fac.source_name;
    target_name = fac.target_name;

    regobjects = fac.regobjects;
}

SmartFactory::~SmartFactory()
{
    for (uint i = 0; i < regobjects.size(); ++i)
    {
    }

    if (own_file and source and !shared)
    {
//        source->Close();
//        delete source;
        source = nullptr;
    }
}

void SmartFactory::validate()
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (!regobjects[i].object)
        {
            std::cerr << "No object at " << regobjects[i].object << std::endl;
            regobjects.erase(regobjects.begin() + i);
            --i;
        }
    }
}

void SmartFactory::listRegisteredObjects() const
{
    std::cout << "List of registered objects in " << fac_name << std::endl;
    std::cout << " Number of objects: " << regobjects.size() << std::endl;
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        std::cout << "* " << regobjects[i].object->GetName() << " [ "
                  << regobjects[i].raw_name << " ] "
                  << " [ " << regobjects[i].reg_name << " ] "
                  << " : " << regobjects[i].object->ClassName() << " at "
                  << regobjects[i].object << std::endl;
    }
}

bool SmartFactory::write(TFile* f, bool verbose)
{
    if (!f->IsOpen())
    {
        std::cerr << "File " << f->GetName() << " could not be open!"
                  << std::endl;
        return false;
    }

    validate();

    f->cd();

    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        std::string hname;
        std::string dir;
        splitDir(regobjects[i].reg_name, hname, dir);
        cdDir(f, dir.c_str());

        if (verbose)
            std::cout << std::left << std::setw(70)
                      << std::string(" + Writing " + regobjects[i].reg_name +
                                     " ... ");

        int res = regobjects[i].object->Write(0, TObject::kOverwrite);

        f->cd();

        if (!res)
        {
            std::cerr << "failed writing " << hname << " to file! Aborting."
                      << std::endl;
            std::exit(EXIT_FAILURE);
        }
        if (verbose) std::cout << " [ done ]" << std::endl;
    }

    return true;
}

bool SmartFactory::write(const char* filename, bool verbose)
{
    TFile* f = new TFile(filename, "RECREATE");

    bool res = write(f, verbose);

    f->Close();
    return res;
}

bool SmartFactory::exportStructure(TFile* f, bool verbose)
{
    if (!f->IsOpen())
    {
        std::cerr << "File " << f->GetName() << " could not be open!"
                  << std::endl;
        return false;
    }

    validate();

    SmartFactoryObj obj;

    std::vector<ObjectData>::const_iterator it = regobjects.begin();
    for (; it != regobjects.end(); ++it)
        obj.objnames.push_back(it->reg_name);

    obj.objects_name = obj_name.c_str();
    obj.directory_name = dir_name.c_str();

    f->cd();

    obj.Write((this->fac_name + "_fac").c_str());

    return write(f, verbose);
}

bool SmartFactory::exportStructure(const char* filename, bool verbose)
{
    TFile* f = new TFile(filename, "RECREATE");

    bool res = exportStructure(f, verbose);

    f->Close();
    return res;
}

bool SmartFactory::importStructure(TFile* f, bool verbose)
{
    if (!f->IsOpen())
    {
        std::cerr << "File " << f->GetName() << " could not be open!"
                  << std::endl;
        return false;
    }

    setSource(f);

    SmartFactoryObj* obj;
    f->GetObject<SmartFactoryObj>((this->fac_name + "_fac").c_str(), obj);
    if (!obj) return false;

    obj_name = obj->objects_name.GetString().Data();
    dir_name = obj->directory_name.GetString().Data();

    for (uint i = 0; i < obj->objnames.size(); ++i)
    {
        f->cd();
        if (verbose) std::cout << "Importing " << obj->objnames[i];

        TObject* o = this->getObject(f, obj->objnames[i].c_str());
        if (o)
        {
            this->RegObject(o);
            if (verbose) std::cout << " [ done ] " << std::endl;
        }
        else
        {
            if (verbose) std::cout << " [ failed ] " << std::endl;
        }
    }

    return true;
}

TFile* SmartFactory::importStructure(const char* filename, bool verbose)
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

TObject* SmartFactory::getObject(const std::string& name,
                                 const std::string& dir) const
{
    std::vector<ObjectData>::const_iterator it = regobjects.begin();
    for (; it != regobjects.end(); ++it)
    {
        if (it->reg_name == name) return it->object;
    }

    if (!source) return nullptr;

    std::string fn = format(name);
    TObject* obj = nullptr;
    if (dir.size())
        obj = getObject(source, fn, dir);
    else
        obj = getObject(source, fn);

    if (obj) return obj;

    return nullptr;
}

TObject* SmartFactory::getObject(TDirectory* srcdir,
                                 const std::string& fullname)
{
    std::string hname;
    std::string dir;

    splitDir(fullname, hname, dir);
    return getObject(srcdir, hname, dir);
}

TObject* SmartFactory::getObject(TDirectory* srcdir, const std::string& name,
                                 const std::string& dir)
{
    static pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&mutex1);
    TObject* ptr = nullptr;

    if (srcdir)
    {
        if (dir.size())
            srcdir->cd(dir.c_str());
        else
            srcdir->cd();

        TDirectory* d = gDirectory;

        d->GetObject(name.c_str(), ptr);
        srcdir->cd();
    }

    pthread_mutex_unlock(&mutex1);
    return ptr;
}

void SmartFactory::splitDir(const std::string& fullname, std::string& name,
                            std::string& dir)
{
    size_t slash_pos = fullname.find_last_of("/");
    if (slash_pos != std::string::npos)
    {
        dir = fullname.substr(0, slash_pos);
        name = fullname.substr(slash_pos + 1, std::string::npos);
    }
    else
    {
        name = fullname;
        dir = "";
    }
}

bool SmartFactory::cdDir(TFile* target, const char* dir, bool automkdir) const
{
    if (!target) return false;

    if (!target->GetDirectory(dir))
    {
        if (automkdir)
            if (target->mkdir(dir, dir)) return target->cd(dir);
    }
    else
    {
        return target->cd(dir);
    }

    return false;
}

TCanvas* SmartFactory::RegCanvas(const char* name, const char* title, int width,
                                 int height)
{
    std::string fulltitle = format(title);
    std::string fullname = format(name);
    std::string hname;
    std::string dir;
    splitDir(fullname, hname, dir);

    // try to get object from file
    TCanvas* c = nullptr; //(TCanvas*)getObject(hname, dir); // FIXME disable
                          // fetching canvases from file
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

    if (c)
    {
        ObjectData od;
        od.raw_name = name;
        od.reg_name = fullname;
        od.object = c;
        regobjects.push_back(od);
    }

    return c;
}

TCanvas* SmartFactory::RegCanvas(const char* name, const char* title, int width,
                                 int height, int divsqr)
{
    TCanvas* c = RegCanvas(name, title, width, height);
    // 	if (!this->source)
    if (!c->GetListOfPrimitives()->GetEntries()) c->DivideSquare(divsqr);
    return c;
}

TCanvas* SmartFactory::RegCanvas(const char* name, const char* title, int width,
                                 int height, int cols, int rows)
{
    TCanvas* c = RegCanvas(name, title, width, height);
    // 	if (!this->source)
    if (!c->GetListOfPrimitives()->GetEntries()) c->Divide(cols, rows);
    return c;
}

TObject* SmartFactory::RegClone(TObject* obj, const std::string& new_name)
{
    std::string fullname = format(new_name);
    std::string hname;
    std::string dir;
    splitDir(fullname, hname, dir);

    TObject* c = obj->Clone(hname.c_str());

    if (c)
    {
        ObjectData od;
        od.raw_name = new_name;
        od.reg_name = fullname;
        od.object = c;
        regobjects.push_back(od);
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

    if (obj)
    {
        ObjectData od;
        od.raw_name = name;
        od.reg_name = fullname;
        od.object = obj;
        regobjects.push_back(od);
    }

    return obj;
}

TObject* SmartFactory::RegObject(TObject* obj)
{
    if (obj)
    {
        ObjectData od;
        od.raw_name = obj->GetName();
        od.reg_name = obj->GetName();
        od.object = obj;
        regobjects.push_back(od);
    }

    return obj;
}

std::string SmartFactory::format(const std::string& name) const
{
    std::string n1 = placeholder(name, "@@@d", dir_name.c_str());
    std::string n2 = placeholder(n1, "@@@a", obj_name.c_str());
    return n2;
}

std::string SmartFactory::placeholder(const std::string& pattern, char c,
                                      const std::string& value)
{
    std::string n = pattern;

    // replace 'c' with "value"

    size_t p = std::string::npos;
    while ((p = n.find_first_of(c)) != std::string::npos)
    {
        n.replace(p, 1, value);
    }
    return n;
}

std::string SmartFactory::placeholder(const std::string& pattern,
                                      const std::string& str,
                                      const std::string& value)
{
    std::string n = pattern;

    // replace "str" with "value"

    size_t p = std::string::npos;
    while ((p = n.find(str)) != std::string::npos)
    {
        n.replace(p, str.length(), value);
    }
    return n;
}

void SmartFactory::rename(const char* newname)
{
    obj_name = newname;
    renameAllObjects();
}

void SmartFactory::chdir(const char* newdir)
{
    dir_name = newdir;
    renameAllObjects();
}

void SmartFactory::renameAllObjects()
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        regobjects[i].reg_name = format(regobjects[i].raw_name);
        std::string hname;
        std::string dir;
        splitDir(regobjects[i].reg_name, hname, dir);

        if (regobjects[i].object->InheritsFrom("TPad"))
            ((TPad*)regobjects[i].object)->SetName(hname.c_str());
        else if (regobjects[i].object->InheritsFrom("TNamed"))
            ((TNamed*)regobjects[i].object)->SetName(hname.c_str());
        else
        {
            std::cerr << "Unknow class for SetName "
                      << regobjects[i].object->ClassName() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
}

SmartFactory& SmartFactory::operator=(const SmartFactory& fac)
{
    // 	fac_name = fac.fac_name;
    obj_name = fac.obj_name;
    dir_name = fac.dir_name;

    regobjects = fac.regobjects;

    // file targets
    source = fac.source;
    target = fac.target;

    source_name = fac.source_name;
    target_name = fac.target_name;

    shared = kTRUE;

    for (size_t i = 0; i < fac.regobjects.size(); ++i)
    {
        if (fac.regobjects[i].object)
        { regobjects[i].object = fac.regobjects[i].object->Clone(); }
    }
    return *this;
}

SmartFactory& SmartFactory::operator+=(const SmartFactory& fa)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        // do this only for histogram
        if (regobjects[i].object->InheritsFrom("TH1"))
        {
            // index for fa factory
            int fa_i = i;

            // if fa.rawnames is smaller than this, or
            // if rawnames values are not the same
            if ((fa.regobjects.size() <= i) or
                (regobjects[i].raw_name != fa.regobjects[i].raw_name))
            {
                // search for correct rawname index
                fa_i = fa.findIndexByRawname(regobjects[i].raw_name);

                // if not found, skip this object
                if (fa_i == -1) continue;
            }

            Bool_t res = ((TH1*)regobjects[i].object)
                             ->Add((TH1*)fa.regobjects[fa_i].object);
            if (!res)
            {
                std::cerr << "Failed adding histogram "
                          << regobjects[i].object->GetName() << std::endl;
            }
        }
    }
    return *this;

    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1") and
            fa.regobjects[i].object)
        {
            Bool_t res = ((TH1*)regobjects[i].object)
                             ->Add((TH1*)fa.regobjects[i].object);
            if (!res)
            {
                std::cerr << "Failed adding histogram "
                          << regobjects[i].object->GetName() << std::endl;
            }
        }
    }
    return *this;
}

SmartFactory& SmartFactory::operator-=(const SmartFactory& fa)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1") and
            fa.regobjects[i].object)
        {
            Bool_t res = ((TH1*)regobjects[i].object)
                             ->Add((TH1*)fa.regobjects[i].object, -1);
            if (!res)
            {
                std::cerr << "Failed subtracting histogram "
                          << regobjects[i].object->GetName() << std::endl;
            }
        }
    }
    return *this;
}

SmartFactory& SmartFactory::operator*=(const SmartFactory& fa)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1") and
            fa.regobjects[i].object)
        {
            Bool_t res = ((TH1*)regobjects[i].object)
                             ->Multiply((TH1*)fa.regobjects[i].object);
            if (!res)
            {
                std::cerr << "Failed multiplying histogram "
                          << regobjects[i].object->GetName() << std::endl;
            }
        }
    }
    return *this;
}

SmartFactory& SmartFactory::operator/=(const SmartFactory& fa)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        // do this only for histogram
        if (regobjects[i].object->InheritsFrom("TH1") and
            fa.regobjects[i].object)
        {
            // index for fa factory
            int fa_i = i;

            // if fa.rawnames is smaller than this, or
            // if rawnames values are not the same
            if ((fa.regobjects.size() <= i) or
                (regobjects[i].raw_name != fa.regobjects[i].raw_name))
            {
                // search for correct rawname index
                fa_i = fa.findIndexByRawname(regobjects[i].raw_name);

                // if not found, skip this object
                if (fa_i == -1) continue;
            }

            // 			const size_t xbins =
            // ((TH1*)regobjects[i].object)->GetNbinsX(); 			const size_t ybins
            // =
            // ((TH1*)regobjects[i].object)->GetNbinsY();

            // 			if (regobjects[i].object->InheritsFrom("TH2") and xbins <
            // 20 and ybins < 20)
            // 			{
            //
            // 				printf("*** %s ( %d x %d ):\n",
            // regobjects[i].object->GetName(), ybins, xbins);
            // printf("-
            // Nominator errors:\n"); 				for (int y = 0; y < ybins;
            // ++y)
            // 				{
            // 					for (int x = 0; x < xbins; ++x)
            // 					{
            // 						printf("\t%g",
            // ((TH1*)regobjects[i].object)->GetBinError(x+1, ybins-y));
            // 					}
            // 					printf("\n");
            // 				}
            // 	// 			printf("\n");
            // 				printf("- DeNominator errors:\n");
            // 				for (int y = 0; y < ybins; ++y)
            // 				{
            // 					for (int x = 0; x < xbins; ++x)
            // 					{
            // 						printf("\t%g",
            // ((TH1*)fa.regobjects[i].object)->GetBinError(x+1, ybins-y));
            // 					}
            // 					printf("\n");
            // 				}
            // 				printf("\n");
            // 			}

            Bool_t res = ((TH1*)regobjects[i].object)
                             ->Divide((TH1*)fa.regobjects[fa_i].object);
            if (!res)
            {
                std::cerr << "Failed dividing histogram "
                          << regobjects[i].object->GetName() << std::endl;
            }
            // 			if (regobjects[i].object->InheritsFrom("TH2") and xbins <
            // 20 and ybins < 20)
            // 			{
            //
            // 				printf("- Result errors:\n");
            // 				for (int y = 0; y < ybins; ++y)
            // 				{
            // 					for (int x = 0; x < xbins; ++x)
            // 					{
            // 						printf("\t%g",
            // ((TH1*)regobjects[i].object)->GetBinError(x+1, ybins-y));
            // 					}
            // 					printf("\n");
            // 				}
            // 			}
        }
    }
    return *this;
}

SmartFactory& SmartFactory::operator*=(Float_t num)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1"))
        { ((TH1*)regobjects[i].object)->Scale(num); }
    }
    return *this;
}

SmartFactory& SmartFactory::operator/=(Float_t num)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1"))
        { ((TH1*)regobjects[i].object)->Scale(1.0 / num); }
    }
    return *this;
}

void SmartFactory::norm(const SmartFactory& fa, bool extended)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1") and
            fa.regobjects[i].object)
        {
            Normalize(((TH1*)regobjects[i].object),
                      ((TH1*)fa.regobjects[i].object), extended);
        }
    }
}

void SmartFactory::norm(Float_t num)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1"))
        {
            Float_t integral_ref = num;
            Float_t integral_cur = ((TH1*)regobjects[i].object)->Integral();

            ((TH1*)regobjects[i].object)->Scale(integral_ref / integral_cur);
        }
    }
}

void SmartFactory::setTitleForAll(const TString& title)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1"))
        { ((TH1*)regobjects[i].object)->SetTitle(title); }
    }
}

void SmartFactory::printCounts() const
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1"))
        {
            printf(" %s %f\n", ((TH1*)regobjects[i].object)->GetName(),
                   ((TH1*)regobjects[i].object)->GetEntries());
        }
    }
}

void SmartFactory::printIntegrals() const
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1"))
        {
            printf(" %s %f\n", ((TH1*)regobjects[i].object)->GetName(),
                   ((TH1*)regobjects[i].object)->Integral());
        }
    }
}

void SmartFactory::callFunctionOnObjects(const SmartFactory* fac,
                                         void (*fun)(TObject* dst,
                                                     const TObject* src))
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (!regobjects[i].object) continue;

        fun(regobjects[i].object, fac->getObject(regobjects[i].raw_name));
    }
}

void SmartFactory::reset()
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1"))
        { ((TH1*)regobjects[i].object)->Reset(); }
    }
}

int SmartFactory::findIndex(TObject* obj) const
{
    for (uint i = 0; i < regobjects.size(); ++i)
        if (regobjects[i].object == obj) return i;

    return -1;
}

TObject* SmartFactory::findObject(int index) const
{
    if (index >= 0 and index < (int)regobjects.size())
        return regobjects[index].object;
    else
        return nullptr;
}

int SmartFactory::findIndexByRawname(const std::string& name) const
{
    for (uint i = 0; i < regobjects.size(); ++i)
        if (name == regobjects[i].raw_name) return i;

    return -1;
}
