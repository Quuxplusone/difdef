
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

class Diff {
    std::vector<DiffLine> v;
public:
    DiffLine &operator[](int i) { return v[i]; }
    const DiffLine &operator[](int i)  const { return v[i]; }
    int size() const { return v.size(); }
    void push_back(const DiffLine &dl) {
        v.push_back(dl);
    }
    void append(const Diff &d) {
        v.insert(v.end(), d.v.begin(), d.v.end());
    }
    void reverse() {
        std::reverse(v.begin(), v.end());
    }
};

Diff diff_two_files(const std::vector<const std::string *> &a,
                    const std::vector<const std::string *> &b);
