
#pragma once

#include <string>
#include <vector>

struct Diff2 {
    struct Line {
        const std::string *text;
        bool in_a;
        bool in_b;
        Line(const std::string *t, bool a, bool b): text(t), in_a(a), in_b(b) { }
    };

  private:
    std::vector<Line> v;

  public:
    Line &operator[](int i) { return v[i]; }
    const Line &operator[](int i)  const { return v[i]; }
    int size() const { return v.size(); }
    void push_back(const Line &dl) {
        v.push_back(dl);
    }
    void append(const Diff2 &d) {
        v.insert(v.end(), d.v.begin(), d.v.end());
    }
    void reverse() {
        std::reverse(v.begin(), v.end());
    }
};

Diff2 diff_two_files(const std::vector<const std::string *> &a,
                    const std::vector<const std::string *> &b);
