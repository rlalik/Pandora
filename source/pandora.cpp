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

#include "pandora.hpp"

#include <TCanvas.h>
#include <TCollection.h>
#include <TDirectory.h>
#include <TF1.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TKey.h>
#include <TObjString.h>
#include <TObject.h>

#include <iomanip>
#include <iostream>

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

namespace pandora
{

namespace
{

template <class Map, class Vector> void fill_map_from_vector(Map m, Vector v)
{
    for (const auto& data : v)
        m.insert(data);
}

template <class Map, class Vector> void fill_vector_from_map(Vector v, Map m)
{
    v.resize(m.size());
    for (const auto& data : m)
        v.push_back(data);
}

}; // namespace
pandora::pandora(const char* name) : box_name(name), source(nullptr), shared(kFALSE), own_file(false) {}

pandora::pandora(const pandora& box)
{
    box_name = box.box_name;

    source = box.source;
    target = box.target;
    source_name = box.source_name;
    target_name = box.target_name;

    regobjects = box.regobjects;
}

pandora::~pandora()
{
    for (uint i = 0; i < regobjects.size(); ++i) {}

    if (own_file and source and !shared)
    {
        //        source->Close();
        //        delete source;
        source = nullptr;
    }
}

void pandora::validate()
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

void pandora::list_registered_objects() const
{
    std::cout << "List of " << regobjects.size() << " registered objects in " << box_name << std::endl;
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        std::cout << "* " << regobjects[i].object->GetName() << " [ " << regobjects[i].raw_name << " ] "
                  << " [ " << regobjects[i].reg_name << " ] "
                  << " : " << regobjects[i].object->ClassName() << " at " << regobjects[i].object << std::endl;
    }
}

bool pandora::write(TFile* f, bool verbose)
{
    if (!f->IsOpen())
    {
        std::cerr << "File " << f->GetName() << " could not be open!" << std::endl;
        return false;
    }

    validate();

    f->cd();

    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        std::string hname;
        std::string dir;
        split_dir(regobjects[i].reg_name, hname, dir);
        cd_dir(f, dir.c_str());

        if (verbose)
            std::cout << std::left << std::setw(70)
                      << (TString(" + Writing ") + regobjects[i].reg_name.c_str() + " ... ");

        int res = regobjects[i].object->Write(0, TObject::kOverwrite);

        f->cd();

        if (!res)
        {
            std::cerr << "failed writing " << hname << " to file! Aborting." << std::endl;
            std::exit(EXIT_FAILURE);
        }
        if (verbose) std::cout << " [ done ]" << std::endl;
    }

    return true;
}

bool pandora::write(const char* filename, bool verbose)
{
    TFile* f = new TFile(filename, "RECREATE");

    bool res = write(f, verbose);

    f->Close();
    return res;
}

bool pandora::export_structure(TFile* f, bool verbose, const char* suffix)
{
    if (!f->IsOpen())
    {
        std::cerr << "File " << f->GetName() << " could not be open!" << std::endl;
        return false;
    }

    validate();

    renameAllObjects();

    pandora_obj obj;

    std::vector<object_data>::const_iterator it = regobjects.begin();
    for (; it != regobjects.end(); ++it)
        obj.objects_names.AddLast(new TObjString(it->reg_name.c_str()));

    placeholders.clear();
    fill_vector_from_map(obj.placeholders, placeholders);

    f->cd();

    obj.Write((this->box_name + suffix).c_str());

    return write(f, verbose);
}

bool pandora::export_structure(const char* filename, bool verbose, const char* suffix)
{
    TFile* f = new TFile(filename, "RECREATE");

    bool res = export_structure(f, verbose, suffix);

    f->Close();
    return res;
}

bool pandora::import_structure(TFile* f, bool verbose, const char* suffix)
{
    if (!f->IsOpen())
    {
        std::cerr << "File " << f->GetName() << " could not be open!" << std::endl;
        return false;
    }

    set_source(f);

    pandora_obj* obj = nullptr;
    f->GetObject<pandora_obj>((this->box_name + suffix).c_str(), obj);
    if (!obj) return false;

    placeholders.clear();
    fill_map_from_vector(placeholders, obj->placeholders);

    for (uint i = 0; i < obj->objects_names.GetEntries(); ++i)
    {
        f->cd();

        TObjString* os = dynamic_cast<TObjString*>(obj->objects_names[i]);
        if (!os) continue;

        if (verbose) std::cout << "Importing " << os->String().Data();

        TObject* o = this->get_object(f, os->String().Data());
        if (o)
        {
            this->reg_object(o);
            if (verbose) std::cout << " [ done ] " << std::endl;
        }
        else
        {
            if (verbose) std::cout << " [ failed ] " << std::endl;
        }
    }

    return true;
}

TFile* pandora::import_structure(const char* filename, bool verbose, const char* suffix)
{
    source = new TFile(filename, "READ");

    bool res = import_structure(source, verbose, suffix);

    if (!res)
    {
        source->Close();
        source = nullptr;
    }

    return source;
}

TObject* pandora::get_object(const std::string& name, const std::string& dir) const
{
    std::vector<object_data>::const_iterator it = regobjects.begin();
    for (; it != regobjects.end(); ++it)
    {
        if (it->reg_name == name) return it->object;
    }

    if (!source) return nullptr;

    std::string fn = apply_placehodlers(name);
    TObject* obj = nullptr;
    if (dir.size())
        obj = get_object(source, fn, dir);
    else
        obj = get_object(source, fn);

    if (obj) return obj;

    return nullptr;
}

TObject* pandora::get_object(TDirectory* srcdir, const std::string& fullname)
{
    std::string hname;
    std::string dir;

    split_dir(fullname, hname, dir);
    return get_object(srcdir, hname, dir);
}

TObject* pandora::get_object(TDirectory* srcdir, const std::string& name, const std::string& dir)
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

void pandora::split_dir(const std::string& fullname, std::string& object, std::string& dir)
{
    size_t slash_pos = fullname.find_last_of("/");
    if (slash_pos != std::string::npos)
    {
        dir = fullname.substr(0, slash_pos);
        object = fullname.substr(slash_pos + 1, std::string::npos);
    }
    else
    {
        object = fullname;
        dir = "";
    }
}

bool pandora::cd_dir(TFile* target, const char* dir, bool automkdir) const
{
    if (!target) return false;

    if (!target->GetDirectory(dir))
    {
        if (automkdir)
            if (target->mkdir(dir, dir)) return target->cd(dir);
    }
    else { return target->cd(dir); }

    return false;
}

TObject* pandora::reg_clone(TObject* obj, const std::string& new_name)
{
    std::string fullname = apply_placehodlers(new_name);
    std::string hname;
    std::string dir;
    split_dir(fullname, hname, dir);

    TObject* c = obj->Clone(hname.c_str());

    if (c)
    {
        object_data od;
        od.raw_name = new_name;
        od.reg_name = fullname;
        od.object = c;
        regobjects.push_back(od);
    }

    return c;
}

TObject* pandora::reg_object(const std::string& name)
{
    std::string fullname = apply_placehodlers(name);
    std::string hname;
    std::string dir;
    split_dir(fullname, hname, dir);

    // try to get object from file
    TObject* obj = get_object(hname, dir);

    if (obj)
    {
        object_data od;
        od.raw_name = name;
        od.reg_name = fullname;
        od.object = obj;
        regobjects.push_back(od);
    }

    return obj;
}

TObject* pandora::reg_object(TObject* obj)
{
    if (obj)
    {
        object_data od;
        od.raw_name = obj->GetName();
        od.reg_name = obj->GetName();
        od.object = obj;
        regobjects.push_back(od);
    }

    return obj;
}

std::string pandora::apply_placehodlers(const std::string& name) const
{
    auto s = name;
    for (const auto& phpair : placeholders)
        s = replace_placeholder(s, phpair.first, phpair.second);
    return s;
}

auto pandora::add_placeholder(std::string pattern, std::string value) -> void
{
    auto it = placeholders.find(pattern);
    if (it != placeholders.end())
        it->second = std::move(value);
    else
        placeholders.insert({std::move(pattern), std::move(value)});
}

auto pandora::list_placeholders() -> void
{
    printf("List of placeholders in box %s\n", box_name.c_str());
    for (const auto& p : placeholders)
        printf("  %s  -->  %s\n", p.first.c_str(), p.second.c_str());
}

std::string pandora::replace_placeholder(const std::string& string, char pattern, const std::string& value)
{
    std::string n = string;

    // replace 'pattern' with "value"

    size_t p = std::string::npos;
    while ((p = n.find_first_of(pattern)) != std::string::npos)
    {
        n.replace(p, 1, value);
    }
    return n;
}

std::string pandora::replace_placeholder(const std::string& string, const std::string& pattern,
                                         const std::string& value)
{
    std::string n = string;

    // replace "pattern" with "value"

    size_t p = std::string::npos;
    while ((p = n.find(pattern)) != std::string::npos)
    {
        n.replace(p, pattern.length(), value);
    }
    return n;
}

void pandora::renameAllObjects()
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        regobjects[i].reg_name = apply_placehodlers(regobjects[i].raw_name);
        std::string hname;
        std::string dir;
        split_dir(regobjects[i].reg_name, hname, dir);

        if (regobjects[i].object->InheritsFrom("TPad"))
            ((TPad*)regobjects[i].object)->SetName(hname.c_str());
        else if (regobjects[i].object->InheritsFrom("TNamed"))
            ((TNamed*)regobjects[i].object)->SetName(hname.c_str());
        else
        {
            std::cerr << "Unknow class for SetName " << regobjects[i].object->ClassName() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }
}

pandora& pandora::operator=(const pandora& box)
{
    // 	box_name = box.box_name;

    regobjects = box.regobjects;
    placeholders = box.placeholders;

    // file targets
    source = box.source;
    target = box.target;

    source_name = box.source_name;
    target_name = box.target_name;

    shared = kTRUE;

    for (size_t i = 0; i < box.regobjects.size(); ++i)
    {
        if (box.regobjects[i].object) { regobjects[i].object = box.regobjects[i].object->Clone(); }
    }
    return *this;
}

pandora& pandora::operator+=(const pandora& fa)
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
            if ((fa.regobjects.size() <= i) or (regobjects[i].raw_name != fa.regobjects[i].raw_name))
            {
                // search for correct rawname index
                fa_i = fa.findIndexByRawname(regobjects[i].raw_name);

                // if not found, skip this object
                if (fa_i == -1) continue;
            }

            Bool_t res = ((TH1*)regobjects[i].object)->Add((TH1*)fa.regobjects[fa_i].object);
            if (!res) { std::cerr << "Failed adding histogram " << regobjects[i].object->GetName() << std::endl; }
        }
    }
    return *this;

    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1") and fa.regobjects[i].object)
        {
            Bool_t res = ((TH1*)regobjects[i].object)->Add((TH1*)fa.regobjects[i].object);
            if (!res) { std::cerr << "Failed adding histogram " << regobjects[i].object->GetName() << std::endl; }
        }
    }
    return *this;
}

pandora& pandora::operator-=(const pandora& fa)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1") and fa.regobjects[i].object)
        {
            Bool_t res = ((TH1*)regobjects[i].object)->Add((TH1*)fa.regobjects[i].object, -1);
            if (!res) { std::cerr << "Failed subtracting histogram " << regobjects[i].object->GetName() << std::endl; }
        }
    }
    return *this;
}

pandora& pandora::operator*=(const pandora& fa)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1") and fa.regobjects[i].object)
        {
            Bool_t res = ((TH1*)regobjects[i].object)->Multiply((TH1*)fa.regobjects[i].object);
            if (!res) { std::cerr << "Failed multiplying histogram " << regobjects[i].object->GetName() << std::endl; }
        }
    }
    return *this;
}

pandora& pandora::operator/=(const pandora& fa)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        // do this only for histogram
        if (regobjects[i].object->InheritsFrom("TH1") and fa.regobjects[i].object)
        {
            // index for fa factory
            int fa_i = i;

            // if fa.rawnames is smaller than this, or
            // if rawnames values are not the same
            if ((fa.regobjects.size() <= i) or (regobjects[i].raw_name != fa.regobjects[i].raw_name))
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

            Bool_t res = ((TH1*)regobjects[i].object)->Divide((TH1*)fa.regobjects[fa_i].object);
            if (!res) { std::cerr << "Failed dividing histogram " << regobjects[i].object->GetName() << std::endl; }
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

pandora& pandora::operator*=(Float_t num)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1")) { ((TH1*)regobjects[i].object)->Scale(num); }
    }
    return *this;
}

pandora& pandora::operator/=(Float_t num)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1")) { ((TH1*)regobjects[i].object)->Scale(1.0 / num); }
    }
    return *this;
}

void pandora::norm(const pandora& fa, bool extended)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1") and fa.regobjects[i].object)
        {
            Normalize(((TH1*)regobjects[i].object), ((TH1*)fa.regobjects[i].object), extended);
        }
    }
}

void pandora::norm(Float_t num)
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

void pandora::set_title_for_all(const TString& title)
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1")) { ((TH1*)regobjects[i].object)->SetTitle(title); }
    }
}

void pandora::print_counts() const
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1"))
        {
            printf(" %s %f\n", ((TH1*)regobjects[i].object)->GetName(), ((TH1*)regobjects[i].object)->GetEntries());
        }
    }
}

void pandora::print_integrals() const
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1"))
        {
            printf(" %s %f\n", ((TH1*)regobjects[i].object)->GetName(), ((TH1*)regobjects[i].object)->Integral());
        }
    }
}

void pandora::call_function_on_objects(const pandora* box, void (*fun)(TObject* dst, const TObject* src))
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (!regobjects[i].object) continue;

        fun(regobjects[i].object, box->get_object(regobjects[i].raw_name));
    }
}

void pandora::reset()
{
    for (size_t i = 0; i < regobjects.size(); ++i)
    {
        if (regobjects[i].object->InheritsFrom("TH1")) { ((TH1*)regobjects[i].object)->Reset(); }
    }
}

int pandora::findIndex(TObject* obj) const
{
    for (uint i = 0; i < regobjects.size(); ++i)
        if (regobjects[i].object == obj) return i;

    return -1;
}

TObject* pandora::findObject(int index) const
{
    if (index >= 0 and index < (int)regobjects.size())
        return regobjects[index].object;
    else
        return nullptr;
}

int pandora::findIndexByRawname(const std::string& name) const
{
    for (uint i = 0; i < regobjects.size(); ++i)
        if (name == regobjects[i].raw_name) return i;

    return -1;
}

}; // namespace pandora
