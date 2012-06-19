
#include <cassert>
#include <vector>

#include "diff2.h"
#include "diffn.h"

/* Static member variable */
int Diff::NUM_FILES = 0;

extern std::vector<const std::string *> lcs_unique(
        const std::vector<const std::string *> &a,
        const std::vector<const std::string *> &b);


Diff diff_three_files(const std::vector<const std::string *> &a,
                      const std::vector<const std::string *> &b,
                      const std::vector<const std::string *> &c)
{
    Diff2 dab = diff_two_files(a,b);
    Diff2 dbc = diff_two_files(b,c);
    Diff2 dac = diff_two_files(a,c);
    int dab_weight = dab.num_common_lines();
    int dbc_weight = dbc.num_common_lines();
    int dac_weight = dac.num_common_lines();
    
    if (dab_weight < dbc_weight) return diff_three_files(b,c,a);
    if (dab_weight < dac_weight) return diff_three_files(a,c,b);

    Diff::NUM_FILES = 3;
    Diff result, suffix;

    int i = 0;
    while (i < dab.size() && i < c.size() && dab[i].text == c[i]) {
        const std::string *line = c[i];
        Diff::Line temp(line, false);
        temp.in_[0] = dab[i].in_a;
        temp.in_[1] = dab[i].in_b;
        temp.in_[2] = true;
        result.push_back(temp);
        ++i;
    }
    int jab = dab.size();
    int jc = c.size();
    while (jab > i && jc > i && dab[jab-1].text == c[jc-1]) {
        const std::string *line = c[jc-1];
        Diff::Line temp(line, false);
        temp.in_[0] = dab[jab-1].in_a;
        temp.in_[1] = dab[jab-1].in_b;
        temp.in_[2] = true;
        suffix.push_back(temp);
        --jab; --jc;
    }
    suffix.reverse();
    /* Make sure [i,jab) and [i,jc) are real ranges. */
    assert(0 <= i && i <= jab && jab <= dab.size());
    assert(0 <= i && i <= jc && jc <= c.size());
    
    /* Now extract all the lines which appear exactly once in "ab" AND once in "c".
     * However, this is not guaranteed to be a common subsequence. */
    std::vector<const std::string *> uab;
    std::vector<const std::string *> uc;
    for (int k = i; k < jab; ++k) {
        const std::string *line = dab[k].text;
        bool failed = false;
        for (int k2 = i; !failed && k2 < jab; ++k2) {
            if (k2 == k) continue;
            if (dab[k2].text == line) failed = true;
        }
        if (failed) continue;
        /* The line appears only once in 'ab'. */
        bool seen = false;
        for (int k2 = i; !failed && k2 < jc; ++k2) {
            if (c[k2] == line) {
                if (seen) failed = true;
                seen = true;
            }
        }
        if (failed || !seen) continue;
        /* The line appears exactly once in 'c', too. */
        uab.push_back(line);
    }

    for (int k = i; k < jc; ++k) {
        const std::string *line = c[k];
        if (std::find(uab.begin(), uab.end(), line) != uab.end())
           uc.push_back(line);
    }
    
    /* Run patience diff on these unique lines. */
    std::vector<const std::string *> ud = lcs_unique(uab, uc);

    if (ud.empty()) {
        /* Base case: There are no unique shared lines between a and b. */
        for (int k = i; k < jab; ++k) {
            Diff::Line temp(dab[k].text, false);
            temp.in_[0] = dab[k].in_a;
            temp.in_[1] = dab[k].in_b;
            result.push_back(temp);
        }
        for (int k = i; k < jc; ++k) {
            Diff::Line temp(c[k], false);
            temp.in_[2] = true;
            result.push_back(temp);
        }
    } else {
        /* Recurse on the interstices. */
        int abk = i;
        int ck = i;
        std::vector<const std::string *> tab;
        std::vector<const std::string *> tc;
        for (int udx = 0; udx < ud.size(); ++udx) {
            assert(abk < jab);
            assert(ck < jc);
            int abk2 = abk;
            while (dab[abk].text != ud[udx]) { tab.push_back(dab[abk].text); ++abk; assert(abk < jab); }
            while (c[ck] != ud[udx]) { tc.push_back(c[ck]); ++ck; assert(ck < jc); }
            Diff2 d = diff_two_files(tab, tc);
            for (int k = 0; k < (int)d.size(); ++k) {
                Diff::Line temp(d[k].text, false);
                if (d[k].in_a) {
                    assert(abk2 < jab);
                    temp.in_[0] = dab[abk2].in_a;
                    temp.in_[1] = dab[abk2].in_b;
                    ++abk2;
                }
                temp.in_[2] = d[k].in_b;
                result.push_back(temp);
            }
            tab.clear();
            tc.clear();
            assert(abk < jab);
            assert(ck < jc);
            assert(dab[abk].text == ud[udx]);
            assert(c[ck] == ud[udx]);
            Diff::Line temp(ud[udx], true);
            temp.in_[0] = dab[abk].in_a;
            temp.in_[1] = dab[abk].in_b;
            result.push_back(temp);
            ++abk;
            ++ck;
        }
        int abk2 = abk;
        while (abk < jab) { tab.push_back(dab[abk].text); ++abk; }
        while (ck < jc) { tc.push_back(c[ck]); ++ck; }
        Diff2 d = diff_two_files(tab, tc);
        for (int k = 0; k < (int)d.size(); ++k) {
            Diff::Line temp(d[k].text, false);
            if (d[k].in_a) {
                assert(abk2 < jab);
                temp.in_[0] = dab[abk2].in_a;
                temp.in_[1] = dab[abk2].in_b;
                ++abk2;
            }
            temp.in_[2] = d[k].in_b;
            result.push_back(temp);
        }
    }

    /* And finally, append the shared suffix. */
    result.append(suffix);
    return result;
}
