
#pragma once

#include <istream>
#include <set>
#include <string>
#include <vector>

class Difdef {
  public:
    typedef unsigned int mask_t;
    static const int MAX_FILES = 32;  // maximum valid NUM_FILES

    const int NUM_FILES;  // set in constructor, read-only
    
    Difdef(int num_files);  // Requires: 0 < num_files <= Difdef::MAX_FILES
    ~Difdef();
    void replace_file(int fileid, std::istream &in);

    struct Diff;
    Diff merge() const;  // merge all N files
    Diff merge(int fileid1, int fileid2) const;  // merge just two files
    Diff merge(const std::set<int> &fileids) const;  // merge a non-empty set of files

    struct Diff {
        struct Line {
            const std::string *text;
            bool in_file(int fileid) const;
            mask_t mask;  // a bitmask

          private:
            Line(): text(NULL), mask(0u) { }
            Line(const std::string *, mask_t);
            friend class Difdef;
            friend class Difdef_impl;
            friend class std::vector<Line>;
        };
        const int dimension;
        std::vector<Line> lines;

        Diff &operator = (const Diff &rhs);
        bool includes_file(int fileid) const;
        mask_t all_files_mask() const;

      private:
        Diff(int num_files, mask_t mask);  // private constructor means you can't create new ones
        void append(const Diff &);
        mask_t mask;
        friend class Difdef_impl;
    };

  private:
    class Difdef_impl *impl;
};
