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

#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "difdef.h"

typedef Difdef::mask_t mask_t;

static void emit_ifdef(mask_t mask)
{
    assert(mask != 0u);
    printf("#if ");
    bool first = true;
    for (int i = 0; i < Difdef::MAX_FILES; ++i) {
        if (mask & ((mask_t)1 << i)) {
            if (!first) printf(" || ");
            printf("defined(V%d)", i+1);
            first = false;
        }
    }
    assert(!first);
    printf("\n");
}

static void emit_endif(mask_t mask)
{
    assert(mask != 0);
    printf("#endif /* ");
    bool first = true;
    for (int i = 0; i < 31; ++i) {
        if (mask & ((mask_t)1 << i)) {
            if (!first) printf(" || ");
            printf("V%d", i+1);
            first = false;
        }
    }
    assert(!first);
    printf(" */\n");
}

/* Let's enforce the following style rules:
 *   (Rule 1) If there are any blank lines in the vicinity of
 *       an #ifdef, place exactly one blank line before the
 *       #ifdef and none after.
 *   (Rule 2) If there are any blank lines in the vicinity of
 *       an #endif, place exactly one blank line after the
 *       #endif and none before.
 * In practice, this is equivalent to:
 *   (Imp 1) Collapse each contiguous series of blank lines into a
 *   single blank line, *unless* the first non-blank line on
 *   both sides have matching masks, in which case simply remove
 *   any blank lines that don't match that mask.
 *   (Imp 2) If a single blank line is preceded by a line with
 *   mask A, and followed by a line with mask B, we want the
 *   blank line's mask to be A|B.
 */
static void collapse_blank_lines(Difdef::Diff &diff)
{
    for (size_t i=0; i < diff.lines.size(); ++i) {
        Difdef::Diff::Line &line = diff.lines[i];
        const bool is_blank_line = (*line.text == "");
        if (!is_blank_line) continue;
        /* Find this series of blank lines. */
        size_t end = i;
        while (end < diff.lines.size() && *diff.lines[end].text == "") ++end;
        /* Look at the lines on either side. */
        mask_t startmask = (i > 0) ? diff.lines[i-1].mask : diff.all_files_mask();
        mask_t endmask = (end < diff.lines.size()) ? diff.lines[end].mask : diff.all_files_mask();
        int blank_lines_we_still_want = 0;
        if (startmask == endmask) {
            /* Preserve these blank lines; they don't border an #ifdef. */
            for (size_t j = i; j < end; ++j) {
                if ((diff.lines[j].mask & startmask) == startmask) {
                    /* This blank line appears in a superset of startmask. */
                    ++blank_lines_we_still_want;
                }
            }
        } else {
            /* There will be an #ifdef here. Reduce N blank lines to 1. */
            blank_lines_we_still_want = 1;
        }
        for (size_t j = i; j < i + blank_lines_we_still_want; ++j) {
            diff.lines[j].mask = (startmask | endmask);
        }
        diff.lines.erase(diff.lines.begin() + i + blank_lines_we_still_want,
                         diff.lines.begin() + end);
        i += blank_lines_we_still_want-1;
    }
}

static void do_print_using_ifdefs(const Difdef::Diff &diff)
{
    std::vector<mask_t> maskstack;
    maskstack.push_back(((mask_t)1 << diff.dimension) - (mask_t)1);  // every file
    for (size_t i=0; i < diff.lines.size(); ++i) {
        const Difdef::Diff::Line &line = diff.lines[i];
        const mask_t mask = line.mask;
        if (mask == maskstack.back()) {
            // we don't need to do anything
        } else {
            while ((mask & maskstack.back()) != mask) {
                // current mask is not yet a superset of mask; keep looking backward
                emit_endif(maskstack.back());
                maskstack.resize(maskstack.size()-1);
                assert(!maskstack.empty());
            }
            if (mask != maskstack.back()) {
                maskstack.push_back(mask);
                emit_ifdef(mask);
            }
        }
        puts(line.text->c_str());
    }
    assert(maskstack.size() >= 1);
    while (maskstack.size() != 1) {
        emit_endif(maskstack.back());
        maskstack.resize(maskstack.size()-1);
    }
}

int main(int argc, char **argv)
{
    bool print_using_ifdefs = false;

    int i;
    for (i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') break;
        if (!strcmp(argv[i], "--")) { ++i; break; }
        if (!strcmp(argv[i], "--ifdefs")) {
            print_using_ifdefs = true;
        } else {
            printf("ERROR: unrecognized option %s\n", argv[i]);
            return 0;
        }
    }

    Difdef difdef(argc-i);
    for (int a = i; a < argc; ++a) {
        std::ifstream f(argv[a]);
        difdef.replace_file(a-i, f);
    }

    Difdef::Diff diff = difdef.merge();

    /* Print out the diff. */
    if (print_using_ifdefs) {
        collapse_blank_lines(diff);
        do_print_using_ifdefs(diff);
    } else {
        for (size_t i=0; i < diff.lines.size(); ++i) {
            const Difdef::Diff::Line &line = diff.lines[i];
            for (int j=0; j < difdef.NUM_FILES; ++j) {
                putchar(line.in_file(j) ? 'a'+j : ' ');
            }
            puts(line.text->c_str());
        }
    }

    return 0;
}