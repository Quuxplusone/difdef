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

static std::set<mask_t> reduce_masks(const std::set<mask_t> &s)
{
    bool indep[Difdef::MAX_FILES][Difdef::MAX_FILES] = {};
    for (int i=0; i < Difdef::MAX_FILES; ++i) {
        mask_t bit_i = (mask_t)1 << i;
        for (int j=0; j < i; ++j) {
            mask_t bit_j = (mask_t)1 << j;
            for (std::set<mask_t>::const_iterator it = s.begin(); it != s.end(); ++it) {
                mask_t mask = *it;
                if (!(mask & bit_i) != !(mask & bit_j)) {
                    indep[i][j] = true;
                    indep[j][i] = true;
                    break;
                }
            }
        }
    }
    std::set<mask_t> result;
    for (int i=0; i < Difdef::MAX_FILES; ++i) {
        mask_t mask = (mask_t)1 << i;
        bool already_added = false;
        for (int j=0; j < Difdef::MAX_FILES; ++j) {
            if (indep[i][j]) continue;
            if (j < i) {
                already_added = true;
            } else {
                mask |= (mask_t)1 << j;
            }
        }
        if (!already_added)
            result.insert(mask);
    }
    return result;
}

void Difdef::Diff::use_synchronization_points(const std::vector<size_t> &points)
{
    /* This routine addresses a problem that we can encounter if the input
     * files contain multi-line comments or string literals.
     * Suppose we have a Diff that looks like this:
     *     ab There is a
     *     a  ro-
     *     ab dent in my car!
     * and we don't want to insert an #endif after "ro-"; perhaps the hyphen
     * in our hypothetical example corresponds to a backslash-continuation
     * or multi-line comment in the real world.
     *   In this case, we generate a sequence of "synchronization points"
     * where it *is* okay to insert #if/#endif directives.
     *     SYNC
     *     ab There is a
     *     SYNC
     *     a  ro-
     *     ab dent in my car!
     *     SYNC
     * Internally, these SYNC points are represented as line numbers; our
     * example would have the vector {0, 1, 3}.  (In fact 0 and N are always
     * required to be sync points.)
     *   This routine considers the Diff as a sequence of mini-Diffs
     * delimited by sync points. Each mini-Diff can be treated trivially
     * by exploding it, if necessary:
     *     a  There is a
     *      b There is a
     *     a  ro-
     *     a  dent in my car!
     *      b dent in my car!
     * The above is a valid (if inefficient) output from this routine.
     *   If every line in one of these mini-Diffs has the same mask, then
     * we don't need to explode it. Contrariwise, if *any* two lines in
     * a mini-Diff have *different* masks, we *must* explode the mini-Diff,
     * because by definition there is no place inside the mini-Diff to
     * insert a pair of #if/#endif directives.
     */
    if (this->lines.empty()) return;

    assert(points[0] == 0);
    assert(points.back() == this->lines.size());

    std::vector<Line> new_lines;
    int spx = 1;
    std::set<mask_t> masks;

    for (size_t i=0; true; ++i) {
        if (i == points[spx]) {
            /* Explode the mini-Diff that just ended. */
            masks = reduce_masks(masks);
            for (std::set<mask_t>::const_iterator it = masks.begin();
                                                  it != masks.end();
                                                  ++it) {
                /* Explode this branch of the mini-Diff. */
                mask_t this_branch = *it;
                for (size_t j = points[spx-1];
                            j < points[spx]; ++j) {
                    if (this->lines[j].mask & this_branch) {
                        assert((this->lines[j].mask & this_branch) == this_branch);
                        const std::string &text = *this->lines[j].text;
                        new_lines.push_back(Difdef::Diff::Line(&text, this_branch));
                    }
                }
            }
            /* All branches have now been exploded. */
            masks.clear();
            ++spx;
        }
        if (i == this->lines.size())
            break;
        masks.insert(this->lines[i].mask);
    }
    
    assert(new_lines.size() >= this->lines.size());
    this->lines = new_lines;
}

void Difdef::Diff::shuffle_identical_masks_together()
{
    /* Given a diff like
     *     a hello
     *      bgoodnight
     *     a sailor
     *      bmoon
     * we want to turn it into
     *     a hello
     *     a sailor
     *      bgoodnight
     *      bmoon
     * This corresponds to a "bubble sort", where two elements can be
     * swapped only if their masks are disjoint, and the sort order is
     * numerically by mask.
     */
    for (size_t s=1; s < this->lines.size(); ++s) {
        /* "s" stands for "source". Find the destination ("d") of this line. */
        size_t d = s;
        for ( ; d > 0; --d) {
            if ((this->lines[s].mask & this->lines[d-1].mask) != 0) {
                /* The two lines are not comparable. Stop. */
                break;
            } else if (this->lines[s].mask > this->lines[d-1].mask) {
                /* The two lines are already in the correct order. */
                break;
            }
        }
        /* Bubble this line up to position "j". */
        assert(d <= s);
        for (size_t k = s; k > d; --k) {
            std::swap(this->lines[k], this->lines[k-1]);
        }
    }
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
            const std::string *s = this->unique_lines.add(fileid, line);
            this->lines[fileid].push_back(s);
        }
    }
}

Difdef::Diff Difdef_impl::merge(unsigned int fmask) const
{
    assert((int)this->lines.size() == this->NUM_FILES);
    assert(0 < this->NUM_FILES && this->NUM_FILES <= Difdef::MAX_FILES);
    assert(fmask != 0u);
    assert(fmask < (1u << this->NUM_FILES));

    Diff d(this->NUM_FILES, 0);
    for (size_t i=0; i < this->lines.size(); ++i) {
        this->add_vec_to_diff(d, i, this->lines[i]);
    }
    return d;
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

Difdef::Diff Difdef::simply_concatenate(const std::vector<std::vector<const std::string *> > &vec)
{
    int num_files = vec.size();
    Diff result(num_files, ((mask_t)1 << num_files) - (mask_t)1);
    for (int v=0; v < num_files; ++v) {
        mask_t vmask = (mask_t)1 << v;
        for (size_t i=0; i < vec[v].size(); ++i) {
            result.lines.push_back(Difdef::Diff::Line(vec[v][i], vmask));
        }
    }
    return result;
}
