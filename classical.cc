
#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "difdef.h"
#include "difdef_impl.h"

typedef std::map<std::pair<int,int>, std::vector<const std::string *> > Memo;

static std::vector<const std::string *> classical_lcs(
        const std::vector<const std::string *> &a,
        const std::vector<const std::string *> &b,
        int i,
        int j,
        Memo &memo)
{
    std::pair<int,int> key(i, j);
    if (memo.find(key) != memo.end()) {
        return memo[key];
    } else if (i == 0 || j == 0) {
        std::vector<const std::string *> result;
        memo[key] = result;
        return result;
    } else if (a[i-1] == b[j-1]) {
        std::vector<const std::string *> result = classical_lcs(a, b, i-1, j-1, memo);
        result.push_back(a[i-1]);
        memo[key] = result;
        return result;
    } else {
        std::vector<const std::string *> result1 = classical_lcs(a, b, i-1, j, memo);
        std::vector<const std::string *> result2 = classical_lcs(a, b, i, j-1, memo);
        if (result1.size() > result2.size()) {
            memo[key] = result1;
            return result1;
        } else {
            memo[key] = result2;
            return result2;
        }
    }
}


void Difdef_impl::add_vec_to_diff_classical(Difdef::Diff &a,
                                            int fileid,
                                            const std::vector<const std::string *> &b) const
{
    assert(this->NUM_FILES == a.dimension);
    assert(0 <= fileid && fileid < a.dimension && a.dimension <= Difdef::MAX_FILES);

    const mask_t bmask = (1u << fileid);

    /* We are guaranteed that the input doesn't have a common prefix or suffix. */
    if (b.empty()) return;
    assert(a.lines.empty() || a.lines[0].text != b[0]);
    assert(a.lines.empty() || a.lines.back().text != b.back());

    std::vector<const std::string *> ta;
    for (size_t i=0; i < a.lines.size(); ++i) {
        ta.push_back(a.lines[i].text);
    }

    Memo memo;
    std::vector<const std::string *> lcs = classical_lcs(ta, b, ta.size(), b.size(), memo);

    Diff result(a.dimension, a.mask | bmask);
    size_t ak = 0;
    size_t bk = 0;
    for (size_t lcx = 0; lcx < lcs.size(); ++lcx) {
        assert(ak < a.lines.size());
        assert(bk < b.size());
        while (a.lines[ak].text != lcs[lcx]) {
            result.lines.push_back(Difdef::Diff::Line(a.lines[ak].text, a.lines[ak].mask));
            ++ak;
        }
        while (b[bk] != lcs[lcx]) {
            result.lines.push_back(Difdef::Diff::Line(b[bk], bmask));
            ++bk;
        }
        assert(a.lines[ak].text == lcs[lcx]);
        assert(b[bk] == lcs[lcx]);
        result.lines.push_back(Difdef::Diff::Line(lcs[lcx], a.lines[ak].mask | bmask));
        ++ak;
        ++bk;
    }
    for ( ; ak < a.lines.size(); ++ak)
        result.lines.push_back(Difdef::Diff::Line(a.lines[ak].text, a.lines[ak].mask));
    for ( ; bk < b.size(); ++bk)
        result.lines.push_back(Difdef::Diff::Line(b[bk], bmask));
    
    /* Now copy the new result into "a". */
    a = result;
}
