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

#include <sys/stat.h>
#include <string>
#include <vector>

#include "difdef.h"

struct FileInfo {
    std::string name;
    struct stat stat;
};

void verify_properly_nested_directives(const Difdef::Diff &diff,
                                       const FileInfo files[]);
bool matches_pp_directive(const std::string &s, const char *directive);
bool matches_if_directive(const std::string &s);
void do_print_using_ifdefs(const Difdef::Diff &diff,
                           const std::vector<std::string> &macro_names,
                           FILE *out);
void do_print_unified_diff(const Difdef::Diff &diff,
                           const FileInfo files[],
                           size_t lines_of_context,
                           FILE *out);
void do_error(const char *fmt, ...);

