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
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stack>
#include <string>
#include <vector>

#include "difdef.h"

typedef Difdef::mask_t mask_t;

static std::vector<std::string> g_MacroNames;

static void do_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
    va_end(ap);
    exit(EXIT_FAILURE);
}

static void emit_ifdef(mask_t mask)
{
    assert(mask != 0u);
    printf("#if ");
    bool first = true;
    for (int i = 0; i < Difdef::MAX_FILES; ++i) {
        if (mask & ((mask_t)1 << i)) {
            if (!first) printf(" || ");
            printf("defined(%s)", g_MacroNames[i].c_str());
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
            printf("%s", g_MacroNames[i].c_str());
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
            /* There will be an #if here. Reduce N blank lines to 1. */
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

static bool matches_pp_directive(const std::string &s, const char *directive)
{
    const int len = strlen(directive);
    const char *p = s.c_str();
    while (isspace(*p)) ++p;
    if (*p++ != '#') return false;
    while (isspace(*p)) ++p;
    return !strncmp(p, directive, len) && (isspace(p[len]) || p[len] == '\0');
}

static bool matches_if_directive(const std::string &s)
{
    return matches_pp_directive(s, "if") ||
           matches_pp_directive(s, "ifdef") ||
           matches_pp_directive(s, "ifndef");
}

void verify_properly_nested_directives(const Difdef::Diff &diff, char *fnames[])
{
    std::vector<std::stack<char> > nest(diff.dimension);
    std::vector<int> lineno(diff.dimension);

    for (size_t i=0; i < diff.lines.size(); ++i) {
        for (int v=0; v < diff.dimension; ++v) {
            if (diff.lines[i].in_file(v))
                lineno[v] += 1;
        }
        const bool is_if = matches_if_directive(*diff.lines[i].text);
        const bool is_elif = matches_pp_directive(*diff.lines[i].text, "elif");
        const bool is_else = matches_pp_directive(*diff.lines[i].text, "else");
        const bool is_endif = matches_pp_directive(*diff.lines[i].text, "endif");
        const bool is_anything = is_if || is_elif || is_else || is_endif;

        if (is_anything) {
            for (int v=0; v < diff.dimension; ++v) {
                if (!diff.lines[i].in_file(v))
                    continue;
                if ((is_elif || is_else || is_endif) && nest[v].empty()) {
                    do_error("ERROR: file %s, line %d: %s with no preceding #if",
                             fnames[v], lineno[v], (is_elif ? "#elif" : is_else ? "#else" : "#endif"));
                }
                if ((is_elif || is_else) && nest[v].top() == 'e') {
                    do_error("ERROR: file %s, line %d: unexpected %s following an #else",
                             fnames[v], lineno[v], (is_elif ? "#elif" : "#else"));
                }
                if (is_if) {
                    nest[v].push('i');
                } else if (is_else) {
                    assert(nest[v].top() == 'i');
                    nest[v].top() = 'e';
                } else if (is_endif) {
                    assert(!nest[v].empty());
                    nest[v].pop();
                }
            }
        }
    }
    for (int v=0; v < diff.dimension; ++v) {
        if (!nest[v].empty()) {
            do_error("ERROR: at end of file %s: expected #endif", fnames[v]);
        }
    }
}

/* For each #if in the file, look up its associated #elif/#else/#endif chain.
 * If every directive in the range has exactly the same mask, and every
 * source line in the range is included in a subset of that mask, then we
 * don't need to do anything. Otherwise, we'll fall back on the guaranteed
 * solution: split out N copies of the entire range, one for each version.
 */
void split_if_elif_ranges_by_version(Difdef::Diff &diff)
{
    for (size_t i=0; i < diff.lines.size(); ++i) {
        if (!matches_if_directive(*diff.lines[i].text))
            continue;

        /* We have an #if. See if its range is all under the same mask. */
        bool need_to_split = false;
        size_t end_of_range = diff.lines.size();
        mask_t desired_mask = diff.lines[i].mask;
        std::vector<std::stack<char> > nest(diff.dimension);
        for (size_t j = i; j < diff.lines.size(); ++j) {
            if (diff.lines[j].mask & ~desired_mask) {
                /* This line's mask is NOT a subset of the desired mask. */
                need_to_split = true;
            }
            const bool is_if = matches_if_directive(*diff.lines[j].text);
            const bool is_elif = matches_pp_directive(*diff.lines[j].text, "elif");
            const bool is_else = matches_pp_directive(*diff.lines[j].text, "else");
            const bool is_endif = matches_pp_directive(*diff.lines[j].text, "endif");
            const bool is_anything = is_if || is_elif || is_else || is_endif;
            if (is_anything && (diff.lines[j].mask != desired_mask)) {
                /* All pp-directives in the range must have the same mask. */
                need_to_split = true;
            }
            if (is_if) {
                for (int v=0; v < diff.dimension; ++v) {
                    if (!diff.lines[j].in_file(v)) continue;
                    nest[v].push('i');
                }
            } else if (is_else) {
                for (int v=0; v < diff.dimension; ++v) {
                    if (!diff.lines[j].in_file(v)) continue;
                    assert(nest[v].top() == 'i');
                    nest[v].top() = 'e';
                }
            } else if (is_endif) {
                bool done = true;
                for (int v=0; v < diff.dimension; ++v) {
                    if (!diff.lines[j].in_file(v)) continue;
                    assert(!nest[v].empty());
                    nest[v].pop();
                    if (!nest[v].empty())
                        done = false;
                }
                if (done) {
                    end_of_range = j+1;
                    break;
                }
            }
        }

        /* The range must contain at least two lines. */
        assert(i+1 < end_of_range && end_of_range <= diff.lines.size());
        assert(matches_if_directive(*diff.lines[i].text));
        assert(matches_pp_directive(*diff.lines[end_of_range-1].text, "endif"));

        if (need_to_split) {
            std::vector<std::vector<const std::string *> > split_versions(diff.dimension);
            for (size_t j = i; j < end_of_range; ++j) {
                for (int v=0; v < diff.dimension; ++v) {
                    if (!diff.lines[j].in_file(v)) continue;
                    split_versions[v].push_back(diff.lines[j].text);
                }
            }
            Difdef::Diff split_merge = Difdef::simply_concatenate(split_versions);
            diff.lines.erase(diff.lines.begin()+i, diff.lines.begin()+end_of_range);
            diff.lines.insert(diff.lines.begin()+i, split_merge.lines.begin(), split_merge.lines.end());
            i += split_merge.lines.size()-1;
        } else {
            i = end_of_range-1;
        }
    }
}


int main(int argc, char **argv)
{
    bool print_using_ifdefs = false;
    std::vector<std::string> user_defined_macro_names;

    int i;
    for (i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') break;
        if (!strcmp(argv[i], "--")) { ++i; break; }
        if (!strcmp(argv[i], "--ifdefs")) {
            print_using_ifdefs = true;
        } else if (argv[i][1] == 'D' && argv[i][2] != '\0') {
            user_defined_macro_names.push_back(argv[i]+2);
            print_using_ifdefs = true;
        } else {
            do_error("ERROR: unrecognized option %s", argv[i]);
        }
    }

    const int num_files = argc - i;
    const int num_user_defined_macros = user_defined_macro_names.size();

    if (num_files == 0) {
        do_error("ERROR: no files provided!");
    }

    if (num_user_defined_macros == 0) {
        char buffer[8];
        for (int j=0; j < num_files; ++j) {
            sprintf(buffer, "V%d", j+1);
            g_MacroNames.push_back(buffer);
        }
    } else if (num_user_defined_macros == num_files) {
        g_MacroNames = user_defined_macro_names;
    } else if (num_user_defined_macros > num_files) {
        do_error("ERROR: %d macro name(s) were provided via -D options, but only %d file(s)!",
                 num_user_defined_macros, num_files);
    } else {
        do_error("ERROR: %d file(s) were provided, but only %d -D option(s)!",
                 num_files, num_user_defined_macros);
    }

    Difdef difdef(num_files);
    for (int a = i; a < argc; ++a) {
        std::ifstream f(argv[a]);
        difdef.replace_file(a-i, f);
    }

    Difdef::Diff diff = difdef.merge();

    /* Print out the diff. */
    if (print_using_ifdefs) {
        verify_properly_nested_directives(diff, argv+i);
        split_if_elif_ranges_by_version(diff);
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