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
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <stack>
#include <string>
#include <vector>

#include <getopt.h>

#include "difdef.h"

typedef Difdef::mask_t mask_t;

static std::vector<std::string> g_MacroNames;
static bool g_PrintUsingIfdefs = false;
static bool g_PrintUnifiedDiff = false;
static size_t g_LinesOfContext = 0;

static void do_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fputs("ERROR: ", stderr);
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
    va_end(ap);
    exit(EXIT_FAILURE);
}

static void do_help()
{
    puts("Usage: diffn [OPTION]... FILE1 [FILE2 FILE3]...");
    puts("Compare or merge multiple files.");
    puts("");
    puts("  -u  -U NUM  --unified[=NUM]  Output NUM (default 3) lines of unified context.");
    puts("  -D NAME                      Output a single merged file using #if syntax.");
    puts("");
    puts("  --help  Output this help.");
    puts("");
    puts("In unified mode, you must supply exactly two files to diff. This mode is");
    puts("intended to be compatible with GNU diff/patch.");
    printf("In \"ifdef\" mode, you may supply 1 <= N <= %d files to merge. The number\n",
        (int)Difdef::MAX_FILES);
    puts("of files must be equal to the number of -D options.");
    printf("In \"raw\" mode (the default), you may supply 1 <= N <= %d files to merge.\n",
        (int)Difdef::MAX_FILES);
    puts("Each line of output will be prefixed by N characters indicating which files");
    puts("contain that line.");
    exit(EXIT_SUCCESS);
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
                    do_error("file %s, line %d: %s with no preceding #if",
                             fnames[v], lineno[v], (is_elif ? "#elif" : is_else ? "#else" : "#endif"));
                }
                if ((is_elif || is_else) && nest[v].top() == 'e') {
                    do_error("file %s, line %d: unexpected %s following an #else",
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
            do_error("at end of file %s: expected #endif", fnames[v]);
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
                /* All top-level pp-directives in the range must have the same mask. */
                for (int v=0; v < diff.dimension; ++v) {
                    if (!diff.lines[j].in_file(v)) continue;
                    if (nest[v].size() == (is_if ? 0 : 1)) {
                        need_to_split = true;
                    }
                }
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
                for (int v=0; v < diff.dimension; ++v) {
                    if (!diff.lines[j].in_file(v)) continue;
                    assert(!nest[v].empty());
                    nest[v].pop();
                }
                bool done = true;
                for (int v=0; v < diff.dimension; ++v) {
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
        }
    }
}


void do_print_unified_diff(const Difdef::Diff &diff, char *fnames[])
{
    assert(fnames[0] != NULL);
    assert(fnames[1] != NULL);

    /* For now we're just faking the diff format's timestamps. */
    char timestamp[64];
    const time_t current_time = time(NULL);
    struct tm const *tm = localtime(&current_time);
    strftime(timestamp, sizeof timestamp, "%Y-%m-%d %H:%M:%S.000000000 %z", tm);
    printf("--- %s\t%s\n", fnames[0], timestamp);
    printf("+++ %s\t%s\n", fnames[1], timestamp);

    size_t abx = 0, ax = 0, bx = 0;
    size_t n = diff.lines.size();
    
  repeat:

    /* Find the first differing line. */
    while (abx < n) {
        if (diff.lines[abx].in_file(0) != diff.lines[abx].in_file(1))
            break;
        ++ax;
        ++bx;
        ++abx;
    }
    
    if (abx == n) return;
    
    assert(diff.lines[abx].in_file(0) != diff.lines[abx].in_file(1));
    assert(ax <= abx && bx <= abx);
    const size_t first_diff_in_ab = abx;
    const size_t first_diff_in_a = ax;
    const size_t first_diff_in_b = bx;
    
    /* Find the last differing line in this hunk. We can subsume
     * non-differing ranges of up to 2*g_LinesOfContext lines. */
    size_t non_differing_range = 0;
    while (abx < n) {
        if (diff.lines[abx].in_file(0) == diff.lines[abx].in_file(1)) {
            if (non_differing_range == 2*g_LinesOfContext) {
                break;
            }
            non_differing_range += 1;
        } else {
            non_differing_range = 0;
        }
        ax += diff.lines[abx].in_file(0);
        bx += diff.lines[abx].in_file(1);
        ++abx;
    }
    
    assert(abx <= n);
    assert(ax <= abx && bx <= abx);
    
    const size_t last_diff_in_ab = (abx - non_differing_range);
    const size_t last_diff_in_a = (ax - non_differing_range);
    const size_t last_diff_in_b = (bx - non_differing_range);
    
    const size_t leading_context = std::min(first_diff_in_ab, g_LinesOfContext);
    const size_t trailing_context = std::min(n - last_diff_in_ab, g_LinesOfContext);

    const size_t hunk_size_in_a =
        leading_context + (last_diff_in_a - first_diff_in_a) + trailing_context;
    const size_t hunk_size_in_b =
        leading_context + (last_diff_in_b - first_diff_in_b) + trailing_context;

    /* Print the line numbers of the hunk. */
    printf("@@ -%d", (int)(first_diff_in_a - leading_context) + (hunk_size_in_a != 0));
    if (hunk_size_in_a != 1) printf(",%d", (int)hunk_size_in_a);
    printf(" +%d", (int)(first_diff_in_b - leading_context) + (hunk_size_in_b != 0));
    if (hunk_size_in_b != 1) printf(",%d", (int)hunk_size_in_b);
    printf(" @@\n");
    
    /* Now print all the lines in the hunk between "start" and "end". */
    for (size_t j = first_diff_in_ab - leading_context;
                j < last_diff_in_ab + trailing_context; ++j) {
        if (diff.lines[j].in_file(0) && diff.lines[j].in_file(1)) {
            putchar(' ');
        } else if (diff.lines[j].in_file(0)) {
            putchar('-');
        } else {
            assert(diff.lines[j].in_file(1));
            putchar('+');
        }
        puts(diff.lines[j].text->c_str());
    }

    /* Any lines we skipped over are either part of the current hunk, or
     * present in both versions (i.e., skipped as part of non_differing_range).
     * Therefore we don't need to "rewind" {abx, ax, bx} at all. */
    goto repeat;
}


int main(int argc, char **argv)
{
    std::vector<std::string> user_defined_macro_names;

    static const struct option longopts[] = {
        { "ifdef", required_argument, NULL, 'D' },
        { "unified", no_argument, NULL, 'u' },
        { "help", no_argument, NULL, 0 },
    };
    int c;
    int longopt_index;
    bool preceded_by_digit = false;
    size_t ocontext = -1;
    while ((c = getopt_long(argc, argv, "0123456789D:uU:", longopts, &longopt_index)) != -1) {
        switch (c) {
            case 0:
                if (!strcmp(longopts[longopt_index].name, "help")) {
                    do_help();
                } else {
                    assert(false);
                }
                break;  /* UNREACHABLE */
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                /* Notice that "diff -u4 -2" has the same behavior
                 * as "diff -u42". This matches GNU. */
                if (preceded_by_digit) {
                    ocontext = 10*ocontext + (c - '0');
                    ocontext = std::min<size_t>(ocontext, 1000);
                } else {
                    ocontext = (c - '0');
                }
                break;
            case 'D':
                g_PrintUsingIfdefs = true;
                user_defined_macro_names.push_back(optarg);
                break;
            case 'U':
            case 'u':
                g_PrintUnifiedDiff = true;
                if (optarg != NULL) {
                    char *end;
                    size_t value = strtoul(optarg, &end, 10);
                    if (end == NULL || *end != '\0') {
                        do_error("invalid context length '%s'", optarg);
                    }
                    g_LinesOfContext = std::max(g_LinesOfContext, value);
                } else {
                    g_LinesOfContext = std::max<size_t>(g_LinesOfContext, 3);
                }
                break;
        }
        preceded_by_digit = isdigit(c);
    }
    
    if (ocontext != (size_t)-1) {
        g_LinesOfContext = ocontext;
    }
    
    assert(optind <= argc);
    const int num_files = argc - optind;
    const int num_user_defined_macros = user_defined_macro_names.size();

    if (g_PrintUnifiedDiff && g_PrintUsingIfdefs) {
        do_error("options --unified/-u/-U and --ifdef/-D are mutually exclusive");
    }

    if (num_files == 0) {
        do_error("no files provided");
    }
    
    if (g_PrintUnifiedDiff && num_files != 2) {
        do_error("unified diff requires exactly two files");
    }
    
    if (g_PrintUsingIfdefs) {
        assert(num_user_defined_macros >= 1);
        if (num_user_defined_macros == num_files) {
            g_MacroNames = user_defined_macro_names;
        } else if (num_user_defined_macros > num_files) {
            do_error("%d macro name(s) were provided via -D options, but only %d file(s)",
                     num_user_defined_macros, num_files);
        } else {
            do_error("%d file(s) were provided, but only %d -D option(s)",
                     num_files, num_user_defined_macros);
        }
    }

    Difdef difdef(num_files);
    for (int i = 0; i < num_files; ++i) {
        std::ifstream f(argv[optind + i]);
        difdef.replace_file(i, f);
    }

    Difdef::Diff diff = difdef.merge();

    /* Print out the diff. */
    if (g_PrintUnifiedDiff) {
        do_print_unified_diff(diff, argv+optind);
    } else if (g_PrintUsingIfdefs) {
        verify_properly_nested_directives(diff, argv+optind);
        split_if_elif_ranges_by_version(diff);
        collapse_blank_lines(diff);
        do_print_using_ifdefs(diff);
    } else {
        /* The default output is a multicolumn format:
         *     a  This line appears only in the first file.
         *     a cThis line appears in files 1 and 3.
         *     abcThis line appears in all three files.
         *      bcThis line appears in files 2 and 3.
         * and so on. This is not very readable, but it is
         * eminently greppable.
         */
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