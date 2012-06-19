
#pragma once

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

struct Diff {
    /* You must set this appropriately before constructing any Diff::Line. */
    static int NUM_FILES;

    struct Line {
        const std::string *text;
        std::vector<bool> in_;
        Line(const std::string *t, Line &l, bool last):
            text(t), in_(l.in_) { in_.push_back(last); assert(in_.size() == Diff::NUM_FILES); }
        Line(const std::string *t, bool l, bool last):
            text(t), in_(Diff::NUM_FILES, l) { in_[Diff::NUM_FILES-1] = last; }
    };

  private:
    std::vector<Line> v;
    int dimension_;  // for sanity-checking only

  public:
    Diff(): dimension_(Diff::NUM_FILES) { }
    Line &operator[](int i) { return v[i]; }
    const Line &operator[](int i)  const { return v[i]; }
    int size() const { return v.size(); }
    int dimension() const { return dimension_; }
    void clear() { v.clear(); }
    void push_back(const Line &dl) {
        assert(dl.in_.size() == this->dimension_);
        v.push_back(dl);
    }
    void append(const Diff &d) {
        assert(d.dimension_ == this->dimension_);
        v.insert(v.end(), d.v.begin(), d.v.end());
    }
    void reverse() {
        std::reverse(v.begin(), v.end());
    }
};

Diff diff_three_files(const std::vector<const std::string *> &a,
                      const std::vector<const std::string *> &b,
                      const std::vector<const std::string *> &c);
