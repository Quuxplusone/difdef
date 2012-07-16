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
#include <cctype>
#include <cstddef>
#include <cstring>
#include <stack>
#include <string>
#include <vector>

#include "diffn.h"


bool matches_pp_directive(const std::string &s, const char *directive)
{
    const int len = strlen(directive);
    const char *p = s.c_str();
    while (isspace(*p)) ++p;
    if (*p++ != '#') return false;
    while (isspace(*p)) ++p;
    return !strncmp(p, directive, len) && (isspace(p[len]) || p[len] == '\0');
}


bool matches_if_directive(const std::string &s)
{
    return matches_pp_directive(s, "if") ||
           matches_pp_directive(s, "ifdef") ||
           matches_pp_directive(s, "ifndef");
}


void verify_properly_nested_directives(const Difdef::Diff &diff, const FileInfo files[])
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
                             files[v].name.c_str(), lineno[v],
                             (is_elif ? "#elif" : is_else ? "#else" : "#endif"));
                }
                if ((is_elif || is_else) && nest[v].top() == 'e') {
                    do_error("file %s, line %d: unexpected %s following an #else",
                             files[v].name.c_str(), lineno[v],
                             (is_elif ? "#elif" : "#else"));
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
            do_error("at end of file %s: expected #endif", files[v].name.c_str());
        }
    }
}
