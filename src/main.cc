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
#include <string>
#include <vector>

#include <getopt.h>

#include "diffn.h"
#include "colors.h"
#include "box_drawing.h"

char *box_drawing_vertical;
char *box_drawing_horizontal;
char *box_drawing_vert_horiz;
char *box_drawing_horiz_vert;
char *box_drawing_right_down;
char *box_drawing_right_up;

typedef Difdef::mask_t mask_t;

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
    puts("Usage: difdef [OPTION]... FILE1 [FILE2 FILE3]...");
    puts("Compare or merge multiple files.");
    puts("");
    puts("  -u -U NUM --unified[=NUM]  Output NUM (default 3) lines of unified context.");
    puts("  -D NAME     --ifdef=NAME   Output a single merged file using #ifdef syntax.");
    puts("  --if EXPR                  As above, but using arbitrary #if syntax.");
    puts("  -D NAME=VALUE              Equivalent to --if NAME==VALUE.");
    puts("      --complex (--simple)   Use (do not use) #elif and #else constructs.");
    puts("  -o  --output=FILE          Write result to FILE instead of standard output.");
    puts("  -r  --recursive            Recursively compare subdirectories.");
    puts("  -t                         Expand tabs and strip trailing whitespace.");
    puts("      --pretty               Set most pretty-printing options.");
    puts("");
    puts("Pretty-printing controls:");
    puts("      --header, --footer     Print filename legend as a header and/or footer.");
    puts("      --lines                Draw lines from columns to filenames.");
    puts("      --separator            Vertically separate left and right columns.");
    puts("      --color[=never|always|auto]");
    puts("                             Use colors: never, always, or (default) on a tty.");
    puts("                             (VTxxx ctlseqs; difdef --color=always | less -R)");
    puts("");
    puts("  --help  Output this help.");
    puts("");
    puts("In unified mode, you must supply exactly two files to diff. This mode is");
    puts("intended to be compatible with GNU diff/patch.");
    printf("In \"ifdef\" mode, you may supply 1 <= N <= %d files to merge. The number\n",
        (int)Difdef::MAX_FILES);
    puts("of files must be equal to the number of -D options. In recursive ifdef mode,");
    puts("you must also supply an output directory name with -o; difdef will duplicate");
    puts("the structure of the input directories in that output directory.");
    printf("In \"raw\" mode (the default), you may supply 1 <= N <= %d files to merge.\n",
        (int)Difdef::MAX_FILES);
    puts("Each line of output will be prefixed by N characters indicating which files");
    puts("contain that line.");
    exit(EXIT_SUCCESS);
}

static void do_print_multicolumn(const Difdef::Diff &diff, FILE *out, int use_colors, int use_separator)
{
    /* The default output is a multicolumn format:
     *     a  This line appears only in the first file.
     *     a cThis line appears in files 1 and 3.
     *     abcThis line appears in all three files.
     *      bcThis line appears in files 2 and 3.
     * and so on. This is not very readable, but it is
     * eminently greppable.
     */
    static const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEF";
    for (size_t i=0; i < diff.lines.size(); ++i) {
        const Difdef::Diff::Line &line = diff.lines[i];
        for (int j=0; j < diff.dimension; ++j) {
            if (get_use_colors(use_colors, out)) {
                fputs(get_term_escape(j), out);
            }
            putc((line.in_file(j) ? alphabet[j] : ' '), out);
        }
        if (get_use_colors(use_colors, out)) {
            fputs(TERM_RESET, out);
        }
        if (use_separator) {
            fputs(box_drawing_vertical, out);
        }
        fprintf(out, "%s\n", line.text->c_str());
    }
}

static void do_print_horizontal_rule(const Difdef::Diff &diff, FILE *out, int use_lines, int use_colors)
{
    for (int col = 0; col < diff.dimension; ++col) {
        if (use_lines) {
            if (get_use_colors(use_colors, out)) {
                fputs(get_term_escape(col), out);
            }
            fputs(box_drawing_vert_horiz, out);
        } else {
            fputs(box_drawing_horizontal, out);
        }
    }
    if (get_use_colors(use_colors, out)) {
        fputs(TERM_RESET, out);
    }
    fputs(box_drawing_vert_horiz, out);
    for (int col = diff.dimension + 1; col < 79; ++col) {
        fputs(box_drawing_horizontal, out);
    }
    fputc('\n', out);
}

static void do_print_legend(const Difdef::Diff &diff,
                            const std::vector<FileInfo> files,
                            FILE *out, int as_header, int use_lines, int use_colors)
{
    static const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEF";
    if (use_lines) {
        if (as_header) {
            for (int row = 0; row < diff.dimension; ++row) {
                for (int col = 0; col < row; ++col) {
                    if (get_use_colors(use_colors, out)) {
                        fputs(get_term_escape(col), out);
                    }
                    fputs(box_drawing_vertical, out);
                }
                if (get_use_colors(use_colors, out)) {
                    fputs(get_term_escape(row), out);
                }
                fputs(box_drawing_right_down, out);
                for (int col = 0; col < diff.dimension - row; ++col) {
                    fputs(box_drawing_horizontal, out);
                }
                fprintf(out, " %c %s", alphabet[row], files[row].name.c_str());
                if (get_use_colors(use_colors, out)) {
                    fputs(TERM_RESET, out);
                }
                fputc('\n', out);
            }
        } else {
            for (int row = diff.dimension - 1; row >= 0; --row) {
                for (int col = 0; col < row; ++col) {
                    if (get_use_colors(use_colors, out)) {
                        fputs(get_term_escape(col), out);
                    }
                    fputs(box_drawing_vertical, out);
                }
                if (get_use_colors(use_colors, out)) {
                    fputs(get_term_escape(row), out);
                }
                fputs(box_drawing_right_up, out);
                for (int col = 0; col < diff.dimension - row; ++col) {
                    fputs(box_drawing_horizontal, out);
                }
                fprintf(out, " %c %s", alphabet[row], files[row].name.c_str());
                if (get_use_colors(use_colors, out)) {
                    fputs(TERM_RESET, out);
                }
                fputc('\n', out);
            }
        }
    } else {
        for (int row = 0; row < diff.dimension; ++row) {
            if (get_use_colors(use_colors, out)) {
                fputs(get_term_escape(row), out);
            }
            fprintf(out, "%c %s", alphabet[row], files[row].name.c_str());
            if (get_use_colors(use_colors, out)) {
                fputs(TERM_RESET, out);
            }
            fputc('\n', out);
        }
    }
}

static std::string do_normalize_whitespace(const std::string &line)
{
    size_t n = line.length();
    bool has_trailing_whitespace = isspace(line[n-1]);
    bool has_tabs = (strchr(line.c_str(), '\t') != NULL);
    if (!has_tabs && !has_trailing_whitespace) {
        return line;
    } else {
        for (--n; isspace(line[n-1]); --n) ;
        std::string result(line, 0, n);
        while (size_t tab = result.find('\t')+1) {
            size_t tab_length = 8 - ((tab-1) % 8);
            result.replace(tab-1, 1, tab_length, ' ');
        }
        return result;
    }
}

int main(int argc, char **argv)
{
    std::vector<std::string> user_defined_macro_names;
    const char *output_filename = NULL;
    bool print_using_ifdefs = false;
    bool print_unified_diff = false;
    bool print_recursively = false;
    bool use_only_simple_ifs = true;
    bool normalize_whitespace = false;
    bool use_header = false;
    bool use_footer = false;
    bool use_lines = false;
    bool use_separator = false;
    bool use_colors_not_specified = true;
    int use_colors = 0;
    size_t lines_of_context = 0;

    static const struct option longopts[] = {
        { "complex", no_argument, NULL, 0 },
        { "if", required_argument, NULL, 0 },
        { "ifdef", required_argument, NULL, 'D' },
        { "output", required_argument, NULL, 'o' },
        { "recursive", no_argument, NULL, 'r' },
        { "simple", no_argument, NULL, 0 },
        { "unified", no_argument, NULL, 'u' },
        { "help", no_argument, NULL, 0 },
        { "header", no_argument, NULL, 0 },
        { "footer", no_argument, NULL, 0 },
        { "lines", no_argument, NULL, 0 },
        { "separator", no_argument, NULL, 0 },
        { "pretty", no_argument, NULL, 0 },
        { "color", optional_argument, NULL, 0 },
        { "protanomaly", no_argument, NULL, 0 },
        { 0, 0, 0, 0 }
    };
    int c;
    int longopt_index;
    bool preceded_by_digit = false;
    size_t ocontext = -1;
    while ((c = getopt_long(argc, argv, "0123456789D:o:rtuU:", longopts, &longopt_index)) != -1) {
        switch (c) {
            case 0:
                if (!strcmp(longopts[longopt_index].name, "help")) {
                    do_help();
                } else if (!strcmp(longopts[longopt_index].name, "if")) {
                    print_using_ifdefs = true;
                    assert(optarg != NULL);
                    user_defined_macro_names.push_back(optarg);
                } else if (!strcmp(longopts[longopt_index].name, "complex")) {
                    use_only_simple_ifs = false;
                } else if (!strcmp(longopts[longopt_index].name, "simple")) {
                    use_only_simple_ifs = true;
                } else if (!strcmp(longopts[longopt_index].name, "header")) {
                    use_header = true;
                } else if (!strcmp(longopts[longopt_index].name, "footer")) {
                    use_footer = true;
                } else if (!strcmp(longopts[longopt_index].name, "lines")) {
                    use_lines = true;
                } else if (!strcmp(longopts[longopt_index].name, "separator")) {
                    use_separator = true;
                } else if (!strcmp(longopts[longopt_index].name, "pretty")) {
                    use_header = true;
                    use_lines = true;
                    use_separator = true;
                    if (use_colors_not_specified) {
                        use_colors = COLOR_AUTO;
                    }
                } else if (!strcmp(longopts[longopt_index].name, "protanomaly")) {
                    set_protanomaly();
                } else if (!strcmp(longopts[longopt_index].name, "color")) {
                    if (!optarg) { // optional argument not specified
                        use_colors_not_specified = false;
                        use_colors = COLOR_AUTO;
                    } else if (!strcmp(optarg, "never")) {
                        use_colors_not_specified = false;
                        use_colors = COLOR_NO;
                    } else if (!strcmp(optarg, "auto")) {
                        use_colors_not_specified = false;
                        use_colors = COLOR_AUTO;
                    } else if (!strcmp(optarg, "always")) {
                        use_colors_not_specified = false;
                        use_colors = COLOR_ALWAYS;
                    } else {
                        fputs("invalid value for --color[=no|auto|always]\n", stderr);
                        exit(EXIT_FAILURE);
                    }
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
            case 'D': {
                print_using_ifdefs = true;
                assert(optarg != NULL);
                const char *equals = strchr(optarg, '=');
                std::string expression;
                if (equals != NULL) {
                    expression = std::string(optarg, equals-optarg)
                               + "==" + std::string(equals+1);
                } else {
                    expression = std::string(BUILTIN_DEFINE) + optarg;
                }
                user_defined_macro_names.push_back(expression);
                break;
            }
            case 'o': {
                assert(optarg != NULL);
                output_filename = optarg;
                break;
            }
            case 'r': {
                print_recursively = true;
                break;
            }
            case 't': {
                normalize_whitespace = true;
                break;
            }
            case 'U':
            case 'u':
                print_unified_diff = true;
                if (optarg != NULL) {
                    char *end;
                    size_t value = strtoul(optarg, &end, 10);
                    if (end == NULL || *end != '\0') {
                        do_error("invalid context length '%s'", optarg);
                    }
                    lines_of_context = std::max(lines_of_context, value);
                } else {
                    lines_of_context = std::max<size_t>(lines_of_context, 3);
                }
                break;
            case '?':
                exit(EXIT_FAILURE);
            default:
                assert(false);
        }
        preceded_by_digit = isdigit(c);
    }

    if (use_separator || use_lines || use_header || use_footer) {
        init_box_drawing(0 /* force ASCII */);
    }

    if (ocontext != (size_t)-1) {
        lines_of_context = ocontext;
    }

    assert(optind <= argc);
    const int num_files = argc - optind;
    const int num_user_defined_macros = user_defined_macro_names.size();

    if (print_unified_diff && print_using_ifdefs) {
        do_error("options --unified/-u/-U and --if/-D are mutually exclusive");
    }

    if (num_files == 0) {
        do_error("no files provided");
    }

    if (print_unified_diff && num_files != 2) {
        do_error("unified diff requires exactly two files");
    }

    if (print_using_ifdefs) {
        assert(num_user_defined_macros >= 1);
        if (num_user_defined_macros == num_files) {
            /* it's okay */
        } else if (num_user_defined_macros > num_files) {
            do_error("%d macro name(s) were provided via -D options, but only %d file(s)",
                     num_user_defined_macros, num_files);
        } else {
            do_error("%d file(s) were provided, but only %d -D option(s)",
                     num_files, num_user_defined_macros);
        }
    }

    if (print_recursively) {
        if (print_using_ifdefs) {
            if (output_filename == NULL) {
                do_error("Recursive #ifdef merge requires an output directory");
            } else if (!strcmp(output_filename, "-")) {
                do_error("Output path '-' is not a directory");
            }
        } else if (print_unified_diff) {
            /* it's okay */
        } else {
            do_error("Recursive diff requires either --ifdef or --unified");
        }
    }

    Difdef difdef(num_files);
    if (normalize_whitespace) {
        difdef.set_filter(do_normalize_whitespace);
    }

    std::vector<FileInfo> files(num_files);

    for (int i=0; i < num_files; ++i) {
        files[i].name = argv[optind + i];
        if (files[i].name == "-") {
            /* Note that "-" always means stdin. If you have a file named
             * "-" in the current directory, you must use "./-". */
            if (print_recursively) {
                do_error("Cannot compare '-' recursively");
            }
            difdef.replace_file(i, stdin);
            fstat(fileno(stdin), &files[i].stat);
        } else {
            const char *fname = files[i].name.c_str();
            FILE *in = fopen(fname, "r");
            if (in == NULL) {
                do_error("Input %s '%s': No such file or directory",
                         print_recursively ? "path" : "file",
                         files[i].name.c_str());
            }
            files[i].fp = in;
            fstat(fileno(in), &files[i].stat);
            const bool is_directory = S_ISDIR(files[i].stat.st_mode);
            if (is_directory && !print_recursively) {
                do_error("Input file '%s' is a directory", fname);
            } else if (print_recursively && !is_directory) {
                do_error("Input path '%s' is not a directory", fname);
            }
            if (!print_recursively) {
                difdef.replace_file(i, in);
                fclose(in);
            }
        }
    }

    if (print_using_ifdefs && print_recursively) {
        /* If we're doing "difdef -r", then files[] is populated with
         * open file descriptors for all the input directories. */
        assert(output_filename != NULL);        
        do_print_ifdefs_recursively(files, user_defined_macro_names,
                                    use_only_simple_ifs, output_filename);
    } else if (print_unified_diff && print_recursively) {
        do_error("Not implemented yet -- TODO FIXME BUG HACK");
    } else {
        /* If we're doing "difdef" without "-r", difdef is populated. */
        Difdef::Diff diff = difdef.merge();

        /* Try to open the output file. */
        FILE *out = stdout;
        if (output_filename != NULL) {
            if (!strcmp(output_filename, "-")) {
                /* Explicitly write to stdout. */
            } else {
                out = fopen(output_filename, "w");
                if (out == NULL) {
                    do_error("Output file '%s': Cannot create file", output_filename);
                }
            }
        }

        /* Print out the diff. */
        if (print_unified_diff) {
            do_print_unified_diff(diff, &files[0], lines_of_context, out);
        } else if (print_using_ifdefs) {
            verify_properly_nested_directives(diff, &files[0]);
            do_print_using_ifdefs(diff, user_defined_macro_names,
                                  use_only_simple_ifs, out);
        } else {
            if (use_header) {
                do_print_legend(diff, files, out, 1 /* as header */, use_lines, use_colors);
                do_print_horizontal_rule(diff, out, use_lines, use_colors);
            }
            do_print_multicolumn(diff, out, use_colors, use_separator);
            if (use_footer) {
                do_print_horizontal_rule(diff, out, use_lines, use_colors);
                do_print_legend(diff, files, out, 0 /* as footer */, use_lines, use_colors);
            }
        }
    }

    return 0;
}
