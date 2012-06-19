
#include <cassert>
#include <vector>

#include "diffn.h"

namespace Difdef {

int NUM_FILES = 0;

extern std::vector<const std::string *> lcs_unique(
        const std::vector<const std::string *> &a,
        const std::vector<const std::string *> &b);

Diff vec_to_diff1(const std::vector<const std::string *> &a)
{
    Diff result(1);
    const int n = a.size();
    for (int i=0; i < n; ++i) {
        result.push_back(Diff::Line(1, a[i], true, true));
    }
    return result;
}

Diff diff_diff_vs_vec(Diff &a, const std::vector<const std::string *> &b)
{
    int NUM_FILES = a.dimension() + 1;
    Diff result(NUM_FILES);
    Diff suffix(NUM_FILES);

    int i = 0;
    while (i < a.size() && i < b.size() && a[i].text == b[i]) {
        const std::string *line = b[i];
        result.push_back(Diff::Line(NUM_FILES, line, a[i], true));
        ++i;
    }
    int ja = a.size();
    int jb = b.size();
    while (ja > i && jb > i && a[ja-1].text == b[jb-1]) {
        const std::string *line = b[jb-1];
        suffix.push_back(Diff::Line(NUM_FILES, line, a[ja-1], true));
        --ja; --jb;
    }
    suffix.reverse();
    /* Make sure [i,ja) and [i,jb) are real ranges. */
    assert(0 <= i && i <= ja && ja <= a.size());
    assert(0 <= i && i <= jb && jb <= b.size());

    /* Now extract all the lines which appear exactly once in "a" AND once in "b".
     * However, this is not guaranteed to be a common subsequence. */
    std::vector<const std::string *> ua;
    std::vector<const std::string *> ub;
    for (int k = i; k < ja; ++k) {
        const std::string *line = a[k].text;
        bool failed = false;
        for (int k2 = i; !failed && k2 < ja; ++k2) {
            if (k2 == k) continue;
            if (a[k2].text == line) failed = true;
        }
        if (failed) continue;
        /* The line appears only once in 'ab'. */
        bool seen = false;
        for (int k2 = i; !failed && k2 < jb; ++k2) {
            if (b[k2] == line) {
                if (seen) failed = true;
                seen = true;
            }
        }
        if (failed || !seen) continue;
        /* The line appears exactly once in 'c', too. */
        ua.push_back(line);
    }

    for (int k = i; k < jb; ++k) {
        const std::string *line = b[k];
        if (std::find(ua.begin(), ua.end(), line) != ua.end())
           ub.push_back(line);
    }

    /* Run patience diff on these unique lines. */
    std::vector<const std::string *> uab = lcs_unique(ua, ub);

    if (uab.empty()) {
        /* Base case: There are no unique shared lines between a and b. */
        for (int k = i; k < ja; ++k) {
            result.push_back(Diff::Line(NUM_FILES, a[k].text, a[k], false));
        }
        for (int k = i; k < jb; ++k) {
            result.push_back(Diff::Line(NUM_FILES, b[k], false, true));
        }
    } else {
        /* Recurse on the interstices. */
        int ak = i;
        int bk = i;
        Diff ta(NUM_FILES-1);
        std::vector<const std::string *> tb;
        for (int uabx = 0; uabx < uab.size(); ++uabx) {
            assert(ak < ja);
            assert(bk < jb);
            while (a[ak].text != uab[uabx]) { ta.push_back(a[ak]); ++ak; assert(ak < ja); }
            while (b[bk] != uab[uabx]) { tb.push_back(b[bk]); ++bk; assert(bk < jb); }
            result.append(diff_diff_vs_vec(ta, tb));
            ta.clear();
            tb.clear();
            assert(ak < ja);
            assert(bk < jb);
            assert(a[ak].text == uab[uabx]);
            assert(b[bk] == uab[uabx]);
            result.push_back(Diff::Line(NUM_FILES, uab[uabx], a[ak], true));
            ++ak;
            ++bk;
        }
        while (ak < ja) { ta.push_back(a[ak]); ++ak; }
        while (bk < jb) { tb.push_back(b[bk]); ++bk; }
        result.append(diff_diff_vs_vec(ta, tb));
    }

    /* And finally, append the shared suffix. */
    result.append(suffix);
    return result;
}

Diff diff_n_files(const std::vector<const std::vector<const std::string *> *> &v)
{
    assert(v.size() > 0);
    Diff d = vec_to_diff1(*v[0]);
    for (int i=1; i < v.size(); ++i) {
        d = diff_diff_vs_vec(d, *v[i]);
    }
    return d;
}

}