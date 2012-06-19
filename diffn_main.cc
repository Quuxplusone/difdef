
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "difdef.h"

int main(int argc, char **argv)
{
    Difdef difdef(argc-1);

    for (int i=1; i < argc; ++i) {
        std::ifstream f(argv[i]);
        difdef.replace_file(i-1, f);
    }

    Difdef::Diff diff = difdef.merge();

    /* Print out the diff. */
    for (size_t i=0; i < diff.lines.size(); ++i) {
        const Difdef::Diff::Line &line = diff.lines[i];
        for (int j=0; j < difdef.NUM_FILES; ++j) {
            putchar(line.in_file(j) ? 'a'+j : ' ');
        }
        putchar(' ');
        puts(line.text->c_str());
    }

    return 0;
}