/*
 * Copyright (C) 2012 Arthur O'Dwyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
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
    void replace_file(int fileid, FILE *in);

    Diff merge(mask_t fileids_mask) const;  // merge a non-empty set of files

    void add_vec_to_diff(Diff &a, int fileid, const std::vector<const std::string *> &b) const;
    void add_vec_to_diff_classical(Diff &a, int fileid, const std::vector<const std::string *> &b) const;
};
