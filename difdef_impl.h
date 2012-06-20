
#pragma once

#include <istream>
#include <map>
#include <string>
#include <vector>

#include "difdef.h"

struct Difdef_StringSet {
    /* effectively, friend class Difdef_impl; */
    const int NUM_FILES;
    struct Data {
        std::vector<int> in;
    };
    typedef std::map<std::string, Data> unique_lines_type;
    unique_lines_type unique_lines;

    Difdef_StringSet(int num_files): NUM_FILES(num_files) {}

    const std::string *add(int fileid, const std::string &text) {
        unique_lines_type::iterator p = unique_lines.find(text);
        if (p == unique_lines.end()) {
            Data d;
            d.in.resize(this->NUM_FILES);
            d.in[fileid] = 1;
            p = unique_lines.insert(p, unique_lines_type::value_type(text, d));
        } else {
            p->second.in[fileid] += 1;
        }
        return &p->first;
    }

    const Data &lookup(const std::string *text) const {
        unique_lines_type::const_iterator p = unique_lines.find(*text);
        assert(p != unique_lines.end());
        return p->second;
    }
};

struct Difdef_impl {
    const int NUM_FILES;  // set in constructor, read-only
    Difdef_StringSet unique_lines;
    std::vector<std::vector<const std::string *> > lines;
    typedef Difdef::Diff Diff;
    typedef Difdef::mask_t mask_t;
  
    Difdef_impl(int num_files): NUM_FILES(num_files), unique_lines(num_files), lines(num_files) { }

    void replace_file(int fileid, std::istream &in);

    Diff merge(mask_t fileids_mask) const;  // merge a non-empty set of files

    void add_vec_to_diff(Diff &a, int fileid, const std::vector<const std::string *> &b) const;
    void add_vec_to_diff_classical(Diff &a, int fileid, const std::vector<const std::string *> &b) const;
};
