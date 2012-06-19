
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include "diff2.h"

typedef std::set<std::string> stringset;

struct stringfile {
    std::string filename;
    stringset *unique_lines;
    std::vector<const std::string *> lines;
    
    stringfile(std::string fname, stringset &u):
        filename(fname), unique_lines(&u)
    {}
    
    void read_lines();
    int size() const { return lines.size(); }
};

void stringfile::read_lines()
{
    std::ifstream in(this->filename.c_str());
    std::string line;
    while (std::getline(in, line)) {
        std::pair<stringset::iterator, bool> p = this->unique_lines->insert(line);
        this->lines.push_back(&(*p.first));
    }
}


int main(int argc, char **argv)
{
    stringset unique_lines;
    std::vector<stringfile> all_files;
    for (int i=1; i < argc; ++i) {
        stringfile f(argv[i], unique_lines);
        f.read_lines();
        all_files.push_back(f);
    }

    Diff result = diff_two_files(all_files[0].lines, all_files[1].lines);

    /* Print out the diff. */
    const int n = result.size();
    for (int i=0; i < n; ++i) {
        const DiffLine &line = result[i];
        if (line.in_a && line.in_b) {
            putchar(' ');
        } else if (line.in_a) {
            putchar('-');
        } else if (line.in_b) {
            putchar('+');
        } else {
            assert(false);
        }
        puts(line.text->c_str());
    }

    return 0;
}