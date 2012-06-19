
#pragma once

#include <string>
#include <vector>

struct DiffLine {
    const std::string *text;
    bool in_a;
    bool in_b;
    DiffLine(const std::string *t, bool a, bool b):
        text(t), in_a(a), in_b(b) { }
};

struct Diff : public std::vector<DiffLine> {
    void append(const Diff &d) {
        this->insert(this->end(), d.begin(), d.end());
    }
};

Diff diff_two_files(const std::vector<const std::string *> &a,
                    const std::vector<const std::string *> &b);
