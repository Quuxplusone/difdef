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
#include <sys/types.h>
#include <dirent.h>
#include <cassert>
#include <set>
#include <string>
#include <vector>

#include "diffn.h"


void do_print_ifdefs_recursively(std::vector<FileInfo> &files,
                                 const std::vector<std::string> &macro_names,
                                 bool use_only_simple_ifs,
                                 const std::string &output_name)
{
    const size_t num_files = files.size();

    /* If what's in "files" is all regular files, diff them.
     * Otherwise, try stepping through each directory in parallel. */
    FileInfo *sample_regular = NULL;
    FileInfo *sample_directory = NULL;
    for (size_t i=0; i < num_files; ++i) {
        if (files[i].fp == NULL) {
            files[i].fp = fopen(files[i].name.c_str(), "r");
            if (files[i].fp == NULL)
                continue;
        }
        fstat(fileno(files[i].fp), &files[i].stat);
        if (S_ISDIR(files[i].stat.st_mode)) {
            sample_directory = &files[i];
        } else if (S_ISREG(files[i].stat.st_mode)) {
            sample_regular = &files[i];
        }
    }
    
    if (sample_regular != NULL && sample_directory != NULL) {
        do_error("Input paths '%s' and '%s' cannot be merged because one is a directory\n"
                 "and the other is a regular file.\n"
                 "Incomplete output may have been left in the output directory.",
                 sample_directory->name.c_str(), sample_regular->name.c_str());
    }

    if (sample_regular != NULL) {
        /* Let's diff these files! */
        Difdef difdef(num_files);
        for (size_t i=0; i < num_files; ++i) {
            if (files[i].fp == NULL) {
                /* This file couldn't be opened, above. */
                continue;
            }
            assert(!S_ISDIR(files[i].stat.st_mode));
            if (S_ISREG(files[i].stat.st_mode)) {
                difdef.replace_file(i, files[i].fp);
            } else {
                difdef.replace_file(i, NULL);
            }
            fclose(files[i].fp);
        }

        Difdef::Diff diff = difdef.merge();

        /* Try to open the output file. */
        FILE *out = fopen(output_name.c_str(), "w");
        if (out == NULL) {
            do_error("Output file '%s': Cannot create file", output_name.c_str());
        }

        /* Print out the diff. */
        verify_properly_nested_directives(diff, &files[0]);
        do_print_using_ifdefs(diff, macro_names, use_only_simple_ifs, out);
        fclose(out);

    } else {
        /* Recursively diff the contents of these directories. */
        if (mkdir(output_name.c_str(), 0777)) {
            do_error("Output path '%s': Cannot create directory", output_name.c_str());
        }
        std::set<std::string> processed_filenames;
        processed_filenames.insert(".");
        processed_filenames.insert("..");
        for (size_t i=0; i < num_files; ++i) {
            if (files[i].fp == NULL)
                continue;
            DIR *dir = opendir(files[i].name.c_str());
            while (struct dirent *file = readdir(dir)) {
                std::string relative_name = file->d_name;
                if (processed_filenames.find(relative_name) != processed_filenames.end()) {
                    /* This filename was matched and handled earlier. */
                    continue;
                }
                processed_filenames.insert(relative_name);

                std::vector<FileInfo> subfiles(files.size());
                for (size_t j=0; j < num_files; ++j) {
                    subfiles[j].name = files[j].name + "/" + relative_name;
                }
                std::string suboutput_name = output_name + "/" + relative_name;
                do_print_ifdefs_recursively(subfiles, macro_names, use_only_simple_ifs,
                                            suboutput_name);
            }
        }
    }
}
