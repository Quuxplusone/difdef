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
#include <string>
#include <vector>

#include <getopt.h>

#include "diffn.h"

typedef Difdef::mask_t mask_t;

static bool g_PrintUsingIfdefs = false;
std::vector<std::string> g_MacroNames;

static bool g_PrintUnifiedDiff = false;
static size_t g_LinesOfContext = 0;


void do_error(const char *fmt, ...)
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
    std::vector<FileInfo> files(num_files);
    for (int i = 0; i < num_files; ++i) {
        files[i].name = argv[optind + i];
        memset(&files[i].stat, 0, sizeof files[i].stat);
        if (files[i].name == "-") {
            difdef.replace_file(i, stdin);
            fstat(fileno(stdin), &files[i].stat);
        } else {
            FILE *fp = fopen(files[i].name.c_str(), "r");
            fstat(fileno(fp), &files[i].stat);
            difdef.replace_file(i, fp);
            fclose(fp);
        }
    }

    Difdef::Diff diff = difdef.merge();

    /* Print out the diff. */
    if (g_PrintUnifiedDiff) {
        do_print_unified_diff(diff, &files[0], g_LinesOfContext);
    } else if (g_PrintUsingIfdefs) {
        verify_properly_nested_directives(diff, &files[0]);
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