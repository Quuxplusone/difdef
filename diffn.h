
#pragma once

#include <algorithm>
#include <cassert>
#include <map>
#include <string>
#include <vector>

namespace Difdef {

extern int NUM_FILES;

class StringSet {
    struct Data {
        std::vector<int> in;
    };
    typedef std::map<std::string, Data> unique_lines_type;
    unique_lines_type unique_lines;
  public:
    const std::string *add(int fileid, const std::string &text) {
        unique_lines_type::iterator p = unique_lines.find(text);
        if (p == unique_lines.end()) {
            Data d;
            d.in.resize(NUM_FILES);
            d.in[fileid] = 1;
            p = unique_lines.insert(p, unique_lines_type::value_type(text, d));
        } else {
            p->second.in[fileid] += 1;
        }
        return &p->first;
    }
};

struct Diff {
    struct Line {
        const std::string *text;
        std::vector<bool> in_;
        Line(int NUM_FILES, const std::string *t, Line &l, bool last):
            text(t), in_(l.in_) { in_.push_back(last); assert(in_.size() == NUM_FILES); }
        Line(int NUM_FILES, const std::string *t, bool l, bool last):
            text(t), in_(NUM_FILES, l) { in_[NUM_FILES-1] = last; }
    };

  private:
    std::vector<Line> v;
    int dimension_;

  public:
    Diff(int NUM_FILES): dimension_(NUM_FILES) { }
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

Diff diff_n_files(const std::vector<const std::vector<const std::string *> *> &v);

}