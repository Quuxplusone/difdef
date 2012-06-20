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
#include <set>
#include <string>
#include <vector>

class Difdef {
  public:
    typedef unsigned int mask_t;
    static const int MAX_FILES = 32;  // maximum valid NUM_FILES

    const int NUM_FILES;  // set in constructor, read-only
    
    Difdef(int num_files);  // Requires: 0 < num_files <= Difdef::MAX_FILES
    ~Difdef();
    void replace_file(int fileid, std::istream &in);

    struct Diff;
    Diff merge() const;  // merge all N files
    Diff merge(int fileid1, int fileid2) const;  // merge just two files
    Diff merge(const std::set<int> &fileids) const;  // merge a non-empty set of files

    struct Diff {
        struct Line {
            const std::string *text;
            bool in_file(int fileid) const;
            mask_t mask;  // a bitmask

          private:
            Line(): text(NULL), mask(0u) { }
            Line(const std::string *, mask_t);
            friend class Difdef;
            friend class Difdef_impl;
            friend class std::vector<Line>;
        };
        const int dimension;
        std::vector<Line> lines;

        Diff &operator = (const Diff &rhs);
        bool includes_file(int fileid) const;
        mask_t all_files_mask() const;

      private:
        Diff(int num_files, mask_t mask);  // private constructor means you can't create new ones
        void append(const Diff &);
        mask_t mask;
        friend class Difdef_impl;
    };

  private:
    class Difdef_impl *impl;
};
