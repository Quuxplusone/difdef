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

#include <algorithm>
#include <cassert>
#include <vector>

#include "difdef.h"
#include "difdef_impl.h"
#include "getline.h"

typedef Difdef::mask_t mask_t;


/** Patience Diff algorithm implementation *******************************/


#include "patience.cc"


/** Classical (dynamic programming) diff implementation ******************/


#include "classical.cc"


/** Difdef::Diff public class functions **********************************/


Difdef::Diff::Diff(int num_files, mask_t mask): dimension(num_files), mask(mask)
{
    assert(0 < num_files && num_files <= Difdef::MAX_FILES);
    assert(mask < ((mask_t)1 << num_files));
}

Difdef::Diff &Difdef::Diff::operator = (const Difdef::Diff &rhs)
{
    assert(this->dimension == rhs.dimension);
    this->mask = rhs.mask;
    this->lines = rhs.lines;
    return *this;
}

void Difdef::Diff::append(const Difdef::Diff &rhs)
{
    assert(this->dimension == rhs.dimension);
    assert(this->mask == rhs.mask);
    this->lines.insert(this->lines.end(), rhs.lines.begin(), rhs.lines.end());
}

bool Difdef::Diff::includes_file(int fileid) const
{
    assert(0 <= fileid && fileid < this->dimension && this->dimension <= Difdef::MAX_FILES);
    return (this->mask & ((mask_t)1 << fileid)) != 0;
}

mask_t Difdef::Diff::all_files_mask() const
{
    return this->mask;
}


Difdef::Diff::Line::Line(const std::string *text, mask_t mask): text(text), mask(mask)
{
    assert(text != NULL);
    assert(mask != 0);
}

bool Difdef::Diff::Line::in_file(int fileid) const
{
    assert(0 <= fileid && fileid < Difdef::MAX_FILES);
    return (this->mask & ((mask_t)1 << fileid)) != 0;
}

/** DifDef public class wrapper functions ********************************/


Difdef::Difdef(int num_files): NUM_FILES(num_files)
{
    assert(num_files > 0);
    this->impl = new Difdef_impl(num_files);
    assert(this->impl != NULL);
}

Difdef::~Difdef()
{
    delete this->impl;
}

void Difdef::set_filter(std::string (*filter)(const std::string &))
{
    this->impl->filter = filter;
}

void Difdef::replace_file(int fileid, FILE *in)
{
    return this->impl->replace_file(fileid, in);
}

Difdef::Diff Difdef::merge() const
{
    assert(0 < this->NUM_FILES && this->NUM_FILES < Difdef::MAX_FILES);
    mask_t m = ((mask_t)1 << this->NUM_FILES) - (mask_t)1;
    return this->impl->merge(m);
}

Difdef::Diff Difdef::merge(int fileid1, int fileid2) const
{
    assert(0 < this->NUM_FILES && this->NUM_FILES < Difdef::MAX_FILES);
    assert(0 <= fileid1 && fileid1 < this->NUM_FILES);
    assert(0 <= fileid2 && fileid2 < this->NUM_FILES);
    mask_t m = ((mask_t)1 << fileid1) | ((mask_t)1 << fileid2);
    return this->impl->merge(m);
}

Difdef::Diff Difdef::merge(const std::set<int> &fileids) const
{
    assert(0 < this->NUM_FILES && this->NUM_FILES < Difdef::MAX_FILES);
    mask_t m = 0;
    for (std::set<int>::const_iterator it = fileids.begin(); it != fileids.end(); ++it) {
        assert(0 <= *it && *it < this->NUM_FILES);
        m |= ((mask_t)1 << *it);
    }
    return this->impl->merge(m);
}


/** DifDef_impl private class functions **********************************/


void Difdef_impl::replace_file(int fileid, FILE *in)
{
    assert(0 <= fileid && fileid < this->NUM_FILES && this->NUM_FILES <= Difdef::MAX_FILES);
    this->lines[fileid].clear();
    if (in != NULL) {
        std::string line;
        while (getline(in, line)) {
            if (this->filter != NULL)
                line = this->filter(line);
            const std::string *s = this->unique_lines.add(fileid, line);
            this->lines[fileid].push_back(s);
        }
    }
}


/* If this line has a brace in column 1, it's highest-priority.
 * If this line has a brace in column 2, it's next-highest. ...
 * If it's completely blank, it's above average priority.
 * Otherwise, it's low priority.
 */
static int diff_ending_priority(const char *text)
{
    int i = 0;
    while (isspace(text[i])) ++i;
    if (text[i] == '}')
        return std::max(100-i, 10);
    if (text[i] == '\0')
        return 1;
    return 0;
}


static inline bool contains(mask_t mortals, mask_t men)
{
    return !(men & ~mortals);
}


static Difdef::Diff &slide_diff_windows(Difdef::Diff &d)
{
    /* As a special heuristic to produce nice merges for source code in
     * curly-brace languages, let's try to make differing ranges end with
     * curly braces; or, failing that, at least end with a blank line.
     * TODO FIXME: There may be a way to generalize this kludge. */
    const size_t N = d.lines.size();
    size_t last_edge = 0;
    for (size_t i=1; i < N; ++i) {
        if (d.lines[i-1].mask == d.lines[i].mask)
            continue;
        /* There will be a directive inserted between i-1 and i. */
        assert(d.lines[last_edge].mask == d.lines[i-1].mask);
        mask_t inner_mask = d.lines[i-1].mask;
        mask_t outer_mask = d.lines[i].mask;
        if (last_edge != 0 && d.lines[last_edge-1].mask == outer_mask && contains(outer_mask, inner_mask)) {
            /* We can slide both edges either direction, as long as the
             * text matches. */
            size_t window_down = 0;
            size_t window_up = 0;
            while (window_down < std::min(last_edge, i - last_edge) &&
                   d.lines[last_edge-window_down-1].mask == outer_mask &&
                   d.lines[last_edge-window_down-1].text == d.lines[i-window_down-1].text)
                ++window_down;
            while (window_up < std::min(N - i, i - last_edge) &&
                   d.lines[i+window_up].mask == outer_mask &&
                   d.lines[i+window_up].text == d.lines[last_edge+window_up].text)
                ++window_up;
            /* We have a "window" of (window_down + window_up) lines, inside
             * which we can freely "slide" the differing range. Look for a
             * right-curly-brace or blank line inside the window. */
            int max_priority = 0;
            size_t max_priority_edge = 0;
            for (size_t j=0; j < window_down + window_up; ++j) {
                const std::string &text = *d.lines[last_edge - window_down + j].text;
                assert(text == *d.lines[i - window_down + j].text);
                const int priority = diff_ending_priority(text.c_str());
                if (priority > max_priority) {
                    max_priority = priority;
                    max_priority_edge = j+1;
                }
            }
            for (size_t j=0; j < max_priority_edge; ++j) {
                d.lines[i - window_down + j].mask = inner_mask;
                d.lines[last_edge - window_down + j].mask = outer_mask;
            }
            for (size_t j = max_priority_edge; j < window_down + window_up; ++j) {
                d.lines[i - window_down + j].mask = outer_mask;
                d.lines[last_edge - window_down + j].mask = inner_mask;
            }
            last_edge = i - window_down + max_priority_edge;
            assert(last_edge == 0 || d.lines[last_edge-1].mask == inner_mask);
            assert(d.lines[last_edge].mask == outer_mask);
            i = std::max(last_edge, i+window_up-1);
        } else {
            last_edge = i;
        }
    }
    
    return d;
}


Difdef::Diff Difdef_impl::merge(unsigned int fmask) const
{
    assert(this->lines.size() == (size_t)this->NUM_FILES);
    assert(0 < this->NUM_FILES && this->NUM_FILES <= Difdef::MAX_FILES);
    assert(fmask != 0);
    assert(fmask < ((mask_t)1 << this->NUM_FILES));

    Diff d(this->NUM_FILES, 0);
    for (size_t i=0; i < this->lines.size(); ++i) {
        this->add_vec_to_diff(d, i, this->lines[i]);
    }
    
    return slide_diff_windows(d);
}


void Difdef_impl::add_vec_to_diff(Difdef::Diff &a, int fileid, const std::vector<const std::string *> &b) const
{
    assert(this->NUM_FILES == a.dimension);
    assert(0 <= fileid && fileid < a.dimension && a.dimension <= Difdef::MAX_FILES);

    const mask_t bmask = (1u << fileid);
    assert((a.mask & bmask) == 0);
    Difdef::Diff result(a.dimension, a.mask | bmask);
    Difdef::Diff suffix(a.dimension, a.mask | bmask);

    /* Record the common prefix. */
    size_t i = 0;
    while (i < a.lines.size() && i < b.size() && a.lines[i].text == b[i]) {
        const std::string *line = b[i];
        result.lines.push_back(Difdef::Diff::Line(line, a.lines[i].mask | bmask));
        ++i;
    }

    const size_t ja = a.lines.size();
    const size_t jb = b.size();

    /* Now extract all the lines which appear exactly once in "a" AND once in "b".
     * However, this is not guaranteed to be a common subsequence. */
    std::vector<const std::string *> ua;
    std::vector<const std::string *> ub;
    for (size_t k = i; k < ja; ++k) {
        const std::string *line = a.lines[k].text;
        const Difdef_StringSet::Data &d = this->unique_lines.lookup(line);
        /* We're looking for lines that appear uniquely in "b", and also in the
         * merged file that is "a". */
        bool failed = (d.in[fileid] == 0);  /* appears nowhere in "b" */
        if (failed) continue;
        /* We still need to make sure that "line" is unique in the merged "a".
         * The only way to do that is to search for it. */
        for (size_t k2 = i; !failed && k2 < ja; ++k2) {
            if (k2 == k) continue;
            if (a.lines[k2].text == line) failed = true;
        }
        if (failed) continue;
        /* Okay, the line appears exactly once in this subrange of "a". */
        bool found = false;
        for (size_t k2 = i; !failed && k2 < jb; ++k2) {
            if (b[k2] == line) {
                if (found) failed = true;
                found = true;
            }
        }
        if (failed || !found) continue;
        ua.push_back(line);
    }

    for (size_t k = i; k < jb; ++k) {
        const std::string *line = b[k];
        if (std::find(ua.begin(), ua.end(), line) != ua.end())
           ub.push_back(line);
    }

    /* Run patience diff on these unique lines. */
    std::vector<const std::string *> lcs = patience_unique_lcs(ua, ub);

    if (lcs.empty()) {
        /* Base case: There are no unique shared lines between a and b.
         * In this case we want to fall back on the classical algorithm. */
        Diff ta(a.dimension, a.mask | bmask);
        std::vector<const std::string *> tb(b.begin() + i, b.begin() + jb);
        for (size_t k = i; k < ja; ++k) {
            ta.lines.push_back(Difdef::Diff::Line(a.lines[k].text, a.lines[k].mask));
        }
        this->add_vec_to_diff_classical(ta, fileid, tb);
        result.append(ta);
    } else {
        /* Recurse on the interstices. */
        size_t ak = i;
        size_t bk = i;
        Diff ta(a.dimension, a.mask);
        std::vector<const std::string *> tb;
        for (size_t lcx = 0; lcx < lcs.size(); ++lcx) {
            assert(ak < ja);
            assert(bk < jb);
            while (a.lines[ak].text != lcs[lcx]) { ta.lines.push_back(a.lines[ak]); ++ak; assert(ak < ja); }
            while (b[bk] != lcs[lcx]) { tb.push_back(b[bk]); ++bk; assert(bk < jb); }
            ta.mask = a.mask;
            this->add_vec_to_diff(ta, fileid, tb);
            result.append(ta);
            ta.lines.clear();
            tb.clear();
            assert(ak < ja);
            assert(bk < jb);
            assert(a.lines[ak].text == lcs[lcx]);
            assert(b[bk] == lcs[lcx]);
            result.lines.push_back(Difdef::Diff::Line(lcs[lcx], a.lines[ak].mask | bmask));
            ++ak;
            ++bk;
        }
        while (ak < ja) { ta.lines.push_back(a.lines[ak]); ++ak; }
        while (bk < jb) { tb.push_back(b[bk]); ++bk; }
        ta.mask = a.mask;
        this->add_vec_to_diff(ta, fileid, tb);
        result.append(ta);
    }

    /* Now copy the new result into "a". */
    a = result;
}


static bool are_equal(const std::vector<const std::string *> &a,
                      const std::vector<const std::string *> &b)
{
    const size_t n = a.size();
    if (b.size() != n) return false;
    for (size_t i=0; i < n; ++i) {
        if (a[i] != b[i]) return false;
    }
    return true;
}


Difdef::Diff Difdef::simply_concatenate(const std::vector<std::vector<const std::string *> > &vec)
{
    int num_files = vec.size();
    mask_t have_handled = 0;
    Diff result(num_files, ((mask_t)1 << num_files) - (mask_t)1);
    for (int v=0; v < num_files; ++v) {
        mask_t vmask = (mask_t)1 << v;
        if (have_handled & vmask) continue;
        for (int w = v; w < num_files; ++w) {
            const mask_t wmask = (mask_t)1 << w;
            if (have_handled & wmask) continue;
            if (are_equal(vec[v], vec[w]))
                vmask |= wmask;
        }
        for (size_t i=0; i < vec[v].size(); ++i) {
            result.lines.push_back(Difdef::Diff::Line(vec[v][i], vmask));
        }
        have_handled |= vmask;
    }
    return result;
}
