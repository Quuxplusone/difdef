
#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

#include "diff2.h"

struct PatienceNode {
    int value;
    PatienceNode *left;
    PatienceNode *down;
    PatienceNode(int v, PatienceNode *l, PatienceNode *d): value(v), left(l), down(d) { }
    ~PatienceNode() { delete down; }
};

static std::vector<int> patience_sequence(const std::vector<int> &v)
{
    std::vector<int> result;
    if (v.empty())
        return result;

    std::vector<PatienceNode *> top_cards;
    for (int i = 0; i < v.size(); ++i) {
        int val = v[i];
        /* Put "val" into the leftmost pile whose top card is greater than it. */
        bool handled = false;
        for (int j=0; j < top_cards.size(); ++j) {
            if (top_cards[j]->value > val) {
                PatienceNode *left = (j > 0) ? top_cards[j-1] : NULL;
                top_cards[j] = new PatienceNode(val, left, top_cards[j]);
                handled = true;
                break;
            }
        }
        if (!handled) {
            PatienceNode *left = top_cards.empty() ? NULL : top_cards[top_cards.size()-1];
            top_cards.push_back(new PatienceNode(val, left, NULL));
        }
    }

    /* Extract the longest common subsequence. */
    int n = top_cards.size();
    assert(n != 0);
    result.resize(n);
    PatienceNode *p = top_cards[top_cards.size()-1];
    for (int i=0; i < n; ++i) {
        assert(p != NULL);
        result[n-i-1] = p->value;
        p = p->left;
    }
    assert(p == NULL);

    assert(!result.empty());
    assert(result[0] <= result[result.size()-1]);
    return result;
}

std::vector<const std::string *> lcs_unique(const std::vector<const std::string *> &a,
                                      const std::vector<const std::string *> &b)
{
    size_t n = a.size();
    assert(b.size() == n);
    std::vector<int> indices(n);
    for (int i=0; i < n; ++i) {
        int index_of_ai_in_b = std::find(b.begin(), b.end(), a[i]) - b.begin();
        assert(0 <= index_of_ai_in_b && index_of_ai_in_b < n);
        indices[i] = index_of_ai_in_b;
    }
    std::vector<int> ps = patience_sequence(indices);
    std::vector<const std::string *> result;
    for (int i=0; i < ps.size(); ++i) {
        result.push_back(b[ps[i]]);
    }
    return result;
}


Diff diff_two_files(const std::vector<const std::string *> &a, const std::vector<const std::string *> &b)
{
    Diff result, suffix;

    int i = 0;
    while (i < a.size() && i < b.size() && a[i] == b[i]) {
        const std::string *line = a[i];
        result.push_back(DiffLine(line, true, true));
        ++i;
    }
    int ja = a.size();
    int jb = b.size();
    while (ja > i && jb > i && a[ja-1] == b[jb-1]) {
        const std::string *line = a[ja-1];
        suffix.push_back(DiffLine(line, true, true));
        --ja; --jb;
    }
    suffix.reverse();
    /* Make sure [i,ja) and [i,jb) are real ranges. */
    assert(0 <= i && i <= ja && ja <= a.size());
    assert(0 <= i && i <= jb && jb <= b.size());
    
    /* Now extract all the lines which appear exactly once in "a" AND once in "b".
     * However, this is not guaranteed to be a common subsequence; consider what
     * happens if 'a' is "abcade" and 'b' is "edcbe". Then 'ua' will be
     * "bcd" and 'ub' will be "dcb". */
    std::vector<const std::string *> ua;
    std::vector<const std::string *> ub;
    for (int k = i; k < ja; ++k) {
        const std::string *line = a[k];
        bool failed = false;
        for (int k2 = i; !failed && k2 < ja; ++k2) {
            if (k2 == k) continue;
            if (a[k2] == line) failed = true;
        }
        if (failed) continue;
        /* The line appears only once in 'a'. */
        bool seen = false;
        for (int k2 = i; !failed && k2 < jb; ++k2) {
            if (b[k2] == line) {
                if (seen) failed = true;
                seen = true;
            }
        }
        if (failed || !seen) continue;
        /* The line appears exactly once in 'b', too. */
        ua.push_back(line);
    }

    for (int k = i; k < jb; ++k) {
        const std::string *line = b[k];
        if (std::find(ua.begin(), ua.end(), line) != ua.end())
           ub.push_back(line);
    }
    
    /* Run patience diff on these unique lines. */
    std::vector<const std::string *> uc = lcs_unique(ua, ub);

    if (uc.empty()) {
        /* Base case: There are no unique shared lines between a and b. */
        for (int k = i; k < ja; ++k)
            result.push_back(DiffLine(a[k], true, false));
        for (int k = i; k < jb; ++k)
            result.push_back(DiffLine(b[k], false, true));
    } else {
        /* Recurse on the interstices. */
        int ak = i;
        int bk = i;
        std::vector<const std::string *> ta;
        std::vector<const std::string *> tb;
        for (int ucx = 0; ucx < uc.size(); ++ucx) {
            assert(ak < ja);
            assert(bk < jb);
            while (a[ak] != uc[ucx]) { ta.push_back(a[ak]); ++ak; assert(ak < ja); }
            while (b[bk] != uc[ucx]) { tb.push_back(b[bk]); ++bk; assert(bk < jb); }
            result.append(diff_two_files(ta, tb));
            ta.clear();
            tb.clear();
            assert(ak < ja);
            assert(bk < jb);
            assert(a[ak] == uc[ucx]);
            assert(b[bk] == uc[ucx]);
            result.push_back(DiffLine(uc[ucx], true, true));
            ++ak;
            ++bk;
        }
        while (ak < ja) { ta.push_back(a[ak]); ++ak; }
        while (bk < jb) { tb.push_back(b[bk]); ++bk; }
        result.append(diff_two_files(ta, tb));
    }

    /* And finally, append the shared suffix. */
    result.append(suffix);
    return result;
}


