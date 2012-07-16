/*
 * Copyright (C) 2012 Arthur O'Dwyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

namespace {

struct PatienceNode {
    int value;
    PatienceNode *left;
    PatienceNode *down;
    PatienceNode(int v, PatienceNode *l, PatienceNode *d): value(v), left(l), down(d) { }
    ~PatienceNode() { delete down; }
};

std::vector<int> patience_longest_increasing_sequence(const std::vector<int> &v)
{
    std::vector<int> result;
    if (v.empty())
        return result;

    std::vector<PatienceNode *> top_cards;
    for (size_t i = 0; i < v.size(); ++i) {
        int val = v[i];
        /* Put "val" into the leftmost pile whose top card is greater than it. */
        bool handled = false;
        for (size_t j=0; j < top_cards.size(); ++j) {
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
    assert(!top_cards.empty());
    size_t n = top_cards.size();
    result.resize(n);
    PatienceNode *p = top_cards[n-1];
    for (size_t i=0; i < n; ++i) {
        assert(p != NULL);
        result[n-i-1] = p->value;
        p = p->left;
    }
    assert(p == NULL);

    assert(!result.empty());
    assert(result[0] <= result[result.size()-1]);
    return result;
}

std::vector<const std::string *> patience_unique_lcs(
        const std::vector<const std::string *> &a,
        const std::vector<const std::string *> &b)
{
    size_t n = a.size();
    assert(b.size() == n);
    std::vector<int> indices(n);
    for (size_t i=0; i < n; ++i) {
        int index_of_ai_in_b = std::find(b.begin(), b.end(), a[i]) - b.begin();
        assert(0 <= index_of_ai_in_b && index_of_ai_in_b < (int)n);
        indices[i] = index_of_ai_in_b;
    }
    std::vector<int> ps = patience_longest_increasing_sequence(indices);
    std::vector<const std::string *> result;
    for (size_t i=0; i < ps.size(); ++i) {
        result.push_back(b[ps[i]]);
    }
    return result;
}

}