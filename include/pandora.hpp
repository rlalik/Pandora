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

#ifndef RT_PANDORA_H
#define RT_PANDORA_H

#include "Pandora/export.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include <TCanvas.h>
#include <TNamed.h>
#include <TObjArray.h>

class TDirectory;
class TFile;

namespace pandora
{
using PlaceholdersMap = std::unordered_map<std::string, std::string>;

struct PANDORA_EXPORT pandora_obj : public TNamed
{
    pandora_obj() { objects_names.SetOwner(); }

    TObjArray objects_names;
    std::vector<std::pair<std::string, std::string>> placeholders;

    ClassDef(pandora_obj, 1);
};

class PANDORA_EXPORT pandora
{
public:
    pandora(const char* name);
    pandora(const pandora& box);
    virtual ~pandora();

    // operators
    pandora& operator=(const pandora& fa);
    pandora& operator+=(const pandora& fa);
    pandora& operator-=(const pandora& fa);
    pandora& operator*=(const pandora& fa);
    pandora& operator/=(const pandora& fa);

    pandora& operator*=(Float_t num);
    pandora& operator/=(Float_t num);

    virtual std::string name() const { return box_name; }

    // objects creation
    // histograms
    template <typename T, typename... Types>
    auto reg_hist(const char* name, const char* title, Types... arguments) -> T*;

    template <class T> auto reg_graph(const char* name, int points) -> T*;

    // canvases
    template <typename... Types> auto reg_canvas(const char* name, const char* title, Types... arguments) -> TCanvas*;

    // TObject general
    TObject* reg_object(TObject* obj);
    TObject* reg_object(TObject* obj, std::string raw_name);
    TObject* reg_object(const std::string& name);

    // register clone of the object
    TObject* reg_clone(TObject* obj, const std::string& new_name);

    // file targets
    virtual void set_source(TFile* file) { source = file; }
    virtual TFile* get_source() const { return source; }
    virtual void set_target(TFile* file) { target = file; }
    virtual TFile* get_target() const { return target; }

    virtual void set_source_name(const char* filename) { source_name = filename; }
    virtual const char* get_source_name() const { return source_name.c_str(); }
    virtual void set_target_name(const char* filename) { target_name = filename; }
    virtual const char* get_target_name() const { return target_name.c_str(); }

    // validate list of objects
    virtual void validate();

    // print list of objects
    virtual void list_registered_objects() const;

    virtual TObject* get_object(const std::string& raw_name);
    static TObject* get_object(TDirectory* srcdir, const std::string& fullname);
    static TObject* get_object(TDirectory* srcdir, const std::string& name, const std::string& dir);

    virtual bool write(TFile* f /* = nullptr*/, bool verbose = false);
    virtual bool write(const char* filename /* = nullptr*/, bool verbose = false);

    virtual bool export_structure(TFile* target, bool verbose = false, const char* suffix = "_pandora_box");
    virtual bool export_structure(const char* filename, bool verbose = false, const char* suffix = "_pandora_box");

    virtual bool import_structure(TFile* target, bool verbose = false, const char* suffix = "_pandora_box");
    virtual TFile* import_structure(const char* filename, bool verbose = false, const char* suffix = "_pandora_box");

    virtual void set_name(const char* name) { box_name = name; }
    virtual void reset();

    // operation on histograms
    virtual void norm(const pandora& fa, bool extended = false);
    virtual void norm(Float_t num);

    virtual void print_counts() const;
    virtual void print_integrals() const;

    // global modifiers
    virtual void set_title_for_all(const TString& title);
    virtual std::string apply_placehodlers(const std::string& name) const;

    virtual void call_function_on_objects(pandora* box, void (*fun)(TObject* dst, const TObject* src));

    virtual auto find_index(TObject* obj) const -> int;
    virtual auto find_object(int index) const -> TObject*;
    virtual auto find_index_by_raw_name(const std::string& name) const -> int;
    virtual auto find_index_by_fullname(const std::string& fullname) const -> int;
    virtual auto get_raw_name(int index) const -> std::string;

    virtual void set_objects_ownership(bool owner = true) { own_file = owner; }

    auto add_placeholder(std::string pattern, std::string value) -> void;
    auto list_placeholders() -> void;

    static std::string replace_placeholder(const std::string& string, const std::string& pattern,
                                           const std::string& value);
    static std::string replace_placeholder(const std::string& string, char pattern, const std::string& value);

private:
    void operator+(const pandora&){};
    void operator-(const pandora&){};

    void renameAllObjects();

    static void split_dir(const std::string& fullname, std::string& object, std::string& dir);
    bool cd_dir(TFile* target, const char* dir, bool automkdir = true) const;

public:
    struct object_data
    {
        std::string raw_name;
        std::string reg_name;
        TObject* object;
    };

private:
    std::string box_name;

    std::vector<object_data> regobjects;
    PlaceholdersMap placeholders;

    // file targets
    TFile* source;
    TFile* target;
    Bool_t shared;

    std::string source_name;
    std::string target_name;

    bool own_file;
};

template <typename T, typename... Types> T* pandora::reg_hist(const char* name, const char* title, Types... arguments)
{
    std::string fullname = apply_placehodlers(name);
    std::string fulltitle = apply_placehodlers(title);

    // try to get object from file
    T* obj = (T*)get_object(fullname);
    if (!obj)
    {
        std::string objname;
        std::string dir;
        split_dir(fullname, objname, dir);

        obj = new T(objname.c_str(), fulltitle.c_str(), arguments...);
        // if (sumw2 and 0 == h->GetSumw2N()) h->Sumw2();

        if (!obj) return nullptr;

        object_data od;
        od.raw_name = name;
        od.reg_name = fullname;
        od.object = obj;
        regobjects.push_back(od);
    }
    return obj;
}

template <class T> auto pandora::reg_graph(const char* name, int points) -> T*
{
    std::string fullname = apply_placehodlers(name);

    // try to get object from file
    T* obj = (T*)get_object(fullname);
    if (!obj)
    {
        std::string objname;
        std::string dir;
        split_dir(fullname, objname, dir);

        obj = new T(points);
        obj->SetName(objname.c_str());

        if (!obj) return nullptr;

        object_data od;
        od.raw_name = name;
        od.reg_name = fullname;
        od.object = obj;
        regobjects.push_back(od);
    }
    return obj;
}

template <typename... Types>
auto pandora::reg_canvas(const char* name, const char* title, Types... arguments) -> TCanvas*
{
    std::string fulltitle = apply_placehodlers(title);
    std::string fullname = apply_placehodlers(name);

    // try to get object from file
    TCanvas* obj = (TCanvas*)get_object(name); // fetching canvases from file
    if (!obj)
    {
        std::string objname;
        std::string dir;
        split_dir(fullname, objname, dir);

        obj = new TCanvas(objname.c_str(), fulltitle.c_str(), arguments...);
        auto w = std::get<0>(std::forward_as_tuple(arguments...));
        auto h = std::get<1>(std::forward_as_tuple(arguments...));
        obj->SetCanvasSize(w, h);
        // this line for code formatting

        if (!obj) return nullptr;

        object_data od;
        od.raw_name = name;
        od.reg_name = fullname;
        od.object = obj;
        regobjects.push_back(od);
    }

    return obj;
}

}; // namespace pandora

#endif // RT_PANDORA_H
