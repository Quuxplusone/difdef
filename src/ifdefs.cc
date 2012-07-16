
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <stack>
#include <string>
#include <vector>

#include "diffn.h"

typedef Difdef::mask_t mask_t;


static void emit_ifdef(mask_t mask, FILE *out)
{
    assert(mask != 0u);
    fprintf(out, "#if ");
    bool first = true;
    for (int i = 0; i < Difdef::MAX_FILES; ++i) {
        if (mask & ((mask_t)1 << i)) {
            if (!first) fprintf(out, " || ");
            fprintf(out, "defined(%s)", g_MacroNames[i].c_str());
            first = false;
        }
    }
    assert(!first);
    fprintf(out, "\n");
}


static void emit_endif(mask_t mask, FILE *out)
{
    assert(mask != 0);
    fprintf(out, "#endif /* ");
    bool first = true;
    for (int i = 0; i < 31; ++i) {
        if (mask & ((mask_t)1 << i)) {
            if (!first) fprintf(out, " || ");
            fprintf(out, "%s", g_MacroNames[i].c_str());
            first = false;
        }
    }
    assert(!first);
    fprintf(out, " */\n");
}


/* For each #if in the file, look up its associated #elif/#else/#endif chain.
 * If every directive in the range has exactly the same mask, and every
 * source line in the range is included in a subset of that mask, then we
 * don't need to do anything. Otherwise, we'll fall back on the guaranteed
 * solution: split out N copies of the entire range, one for each version.
 */
static void split_if_elif_ranges_by_version(Difdef::Diff &diff)
{
    for (size_t i=0; i < diff.lines.size(); ++i) {
        if (!matches_if_directive(*diff.lines[i].text))
            continue;

        /* We have an #if. See if its range is all under the same mask. */
        bool need_to_split = false;
        size_t end_of_range = diff.lines.size();
        mask_t desired_mask = diff.lines[i].mask;
        std::vector<std::stack<char> > nest(diff.dimension);
        for (size_t j = i; j < diff.lines.size(); ++j) {
            if (diff.lines[j].mask & ~desired_mask) {
                /* This line's mask is NOT a subset of the desired mask. */
                need_to_split = true;
            }
            const bool is_if = matches_if_directive(*diff.lines[j].text);
            const bool is_elif = matches_pp_directive(*diff.lines[j].text, "elif");
            const bool is_else = matches_pp_directive(*diff.lines[j].text, "else");
            const bool is_endif = matches_pp_directive(*diff.lines[j].text, "endif");
            const bool is_anything = is_if || is_elif || is_else || is_endif;
            if (is_anything && (diff.lines[j].mask != desired_mask)) {
                /* All top-level pp-directives in the range must have the same mask. */
                for (int v=0; v < diff.dimension; ++v) {
                    if (!diff.lines[j].in_file(v)) continue;
                    if (nest[v].size() == (is_if ? 0 : 1)) {
                        need_to_split = true;
                    }
                }
            }
            if (is_if) {
                for (int v=0; v < diff.dimension; ++v) {
                    if (!diff.lines[j].in_file(v)) continue;
                    nest[v].push('i');
                }
            } else if (is_else) {
                for (int v=0; v < diff.dimension; ++v) {
                    if (!diff.lines[j].in_file(v)) continue;
                    assert(nest[v].top() == 'i');
                    nest[v].top() = 'e';
                }
            } else if (is_endif) {
                for (int v=0; v < diff.dimension; ++v) {
                    if (!diff.lines[j].in_file(v)) continue;
                    assert(!nest[v].empty());
                    nest[v].pop();
                }
                bool done = true;
                for (int v=0; v < diff.dimension; ++v) {
                    if (!nest[v].empty())
                        done = false;
                }
                if (done) {
                    end_of_range = j+1;
                    break;
                }
            }
        }

        /* The range must contain at least two lines. */
        assert(i+1 < end_of_range && end_of_range <= diff.lines.size());
        assert(matches_if_directive(*diff.lines[i].text));
        assert(matches_pp_directive(*diff.lines[end_of_range-1].text, "endif"));

        if (need_to_split) {
            std::vector<std::vector<const std::string *> > split_versions(diff.dimension);
            for (size_t j = i; j < end_of_range; ++j) {
                for (int v=0; v < diff.dimension; ++v) {
                    if (!diff.lines[j].in_file(v)) continue;
                    split_versions[v].push_back(diff.lines[j].text);
                }
            }
            Difdef::Diff split_merge = Difdef::simply_concatenate(split_versions);
            diff.lines.erase(diff.lines.begin()+i, diff.lines.begin()+end_of_range);
            diff.lines.insert(diff.lines.begin()+i, split_merge.lines.begin(), split_merge.lines.end());
            i += split_merge.lines.size()-1;
        }
    }
}


/* Let's enforce the following style rules:
 *   (Rule 1) If there are any blank lines in the vicinity of
 *       an #ifdef, place exactly one blank line before the
 *       #ifdef and none after.
 *   (Rule 2) If there are any blank lines in the vicinity of
 *       an #endif, place exactly one blank line after the
 *       #endif and none before.
 * In practice, this is equivalent to:
 *   (Imp 1) Collapse each contiguous series of blank lines into a
 *   single blank line, *unless* the first non-blank line on
 *   both sides have matching masks, in which case simply remove
 *   any blank lines that don't match that mask.
 *   (Imp 2) If a single blank line is preceded by a line with
 *   mask A, and followed by a line with mask B, we want the
 *   blank line's mask to be A|B.
 */
static void collapse_blank_lines(Difdef::Diff &diff)
{
    for (size_t i=0; i < diff.lines.size(); ++i) {
        Difdef::Diff::Line &line = diff.lines[i];
        const bool is_blank_line = (*line.text == "");
        if (!is_blank_line) continue;
        /* Find this series of blank lines. */
        size_t end = i;
        while (end < diff.lines.size() && *diff.lines[end].text == "") ++end;
        /* Look at the lines on either side. */
        mask_t startmask = (i > 0) ? diff.lines[i-1].mask : diff.all_files_mask();
        mask_t endmask = (end < diff.lines.size()) ? diff.lines[end].mask : diff.all_files_mask();
        int blank_lines_we_still_want = 0;
        if (startmask == endmask) {
            /* Preserve these blank lines; they don't border an #ifdef. */
            for (size_t j = i; j < end; ++j) {
                if ((diff.lines[j].mask & startmask) == startmask) {
                    /* This blank line appears in a superset of startmask. */
                    ++blank_lines_we_still_want;
                }
            }
        } else {
            /* There will be an #if here. Reduce N blank lines to 1. */
            blank_lines_we_still_want = 1;
        }
        for (size_t j = i; j < i + blank_lines_we_still_want; ++j) {
            diff.lines[j].mask = (startmask | endmask);
        }
        diff.lines.erase(diff.lines.begin() + i + blank_lines_we_still_want,
                         diff.lines.begin() + end);
        i += blank_lines_we_still_want-1;
    }
}


void do_print_using_ifdefs(const Difdef::Diff &diff_, FILE *out)
{
    Difdef::Diff diff(diff_);
    split_if_elif_ranges_by_version(diff);
    collapse_blank_lines(diff);

    std::vector<mask_t> maskstack;
    maskstack.push_back(((mask_t)1 << diff.dimension) - (mask_t)1);  // every file
    for (size_t i=0; i < diff.lines.size(); ++i) {
        const Difdef::Diff::Line &line = diff.lines[i];
        const mask_t mask = line.mask;
        if (mask == maskstack.back()) {
            // we don't need to do anything
        } else {
            while ((mask & maskstack.back()) != mask) {
                // current mask is not yet a superset of mask; keep looking backward
                emit_endif(maskstack.back(), out);
                maskstack.resize(maskstack.size()-1);
                assert(!maskstack.empty());
            }
            if (mask != maskstack.back()) {
                maskstack.push_back(mask);
                emit_ifdef(mask, out);
            }
        }
        fprintf(out, "%s\n", line.text->c_str());
    }
    assert(maskstack.size() >= 1);
    while (maskstack.size() != 1) {
        emit_endif(maskstack.back(), out);
        maskstack.resize(maskstack.size()-1);
    }
}

