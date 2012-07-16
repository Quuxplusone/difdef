
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <ctime>

#include "diffn.h"


void do_print_unified_diff(const Difdef::Diff &diff, const FileInfo files[], size_t lines_of_context)
{
    char timestamp[64];
    strftime(timestamp, sizeof timestamp, "%Y-%m-%d %H:%M:%S.000000000 %z",
             localtime(&files[0].stat.st_mtime));
    printf("--- %s\t%s\n", files[0].name.c_str(), timestamp);
    strftime(timestamp, sizeof timestamp, "%Y-%m-%d %H:%M:%S.000000000 %z",
             localtime(&files[1].stat.st_mtime));
    printf("+++ %s\t%s\n", files[1].name.c_str(), timestamp);

    size_t abx = 0, ax = 0, bx = 0;
    size_t n = diff.lines.size();
    
  repeat:

    /* Find the first differing line. */
    while (abx < n) {
        if (diff.lines[abx].in_file(0) != diff.lines[abx].in_file(1))
            break;
        ++ax;
        ++bx;
        ++abx;
    }
    
    if (abx == n) return;
    
    assert(diff.lines[abx].in_file(0) != diff.lines[abx].in_file(1));
    assert(ax <= abx && bx <= abx);
    const size_t first_diff_in_ab = abx;
    const size_t first_diff_in_a = ax;
    const size_t first_diff_in_b = bx;
    
    /* Find the last differing line in this hunk. We can subsume
     * non-differing ranges of up to 2*lines_of_context lines. */
    size_t non_differing_range = 0;
    while (abx < n) {
        if (diff.lines[abx].in_file(0) == diff.lines[abx].in_file(1)) {
            if (non_differing_range == 2*lines_of_context) {
                break;
            }
            non_differing_range += 1;
        } else {
            non_differing_range = 0;
        }
        ax += diff.lines[abx].in_file(0);
        bx += diff.lines[abx].in_file(1);
        ++abx;
    }
    
    assert(abx <= n);
    assert(ax <= abx && bx <= abx);
    
    const size_t last_diff_in_ab = (abx - non_differing_range);
    const size_t last_diff_in_a = (ax - non_differing_range);
    const size_t last_diff_in_b = (bx - non_differing_range);
    
    const size_t leading_context = std::min(first_diff_in_ab, lines_of_context);
    const size_t trailing_context = std::min(n - last_diff_in_ab, lines_of_context);

    const size_t hunk_size_in_a =
        leading_context + (last_diff_in_a - first_diff_in_a) + trailing_context;
    const size_t hunk_size_in_b =
        leading_context + (last_diff_in_b - first_diff_in_b) + trailing_context;

    /* Print the line numbers of the hunk. */
    printf("@@ -%d", (int)(first_diff_in_a - leading_context) + (hunk_size_in_a != 0));
    if (hunk_size_in_a != 1) printf(",%d", (int)hunk_size_in_a);
    printf(" +%d", (int)(first_diff_in_b - leading_context) + (hunk_size_in_b != 0));
    if (hunk_size_in_b != 1) printf(",%d", (int)hunk_size_in_b);
    printf(" @@\n");
    
    /* Now print all the lines in the hunk between "start" and "end". */
    for (size_t j = first_diff_in_ab - leading_context;
                j < last_diff_in_ab + trailing_context; ++j) {
        if (diff.lines[j].in_file(0) && diff.lines[j].in_file(1)) {
            putchar(' ');
        } else if (diff.lines[j].in_file(0)) {
            putchar('-');
        } else {
            assert(diff.lines[j].in_file(1));
            putchar('+');
        }
        puts(diff.lines[j].text->c_str());
    }

    /* Any lines we skipped over are either part of the current hunk, or
     * present in both versions (i.e., skipped as part of non_differing_range).
     * Therefore we don't need to "rewind" {abx, ax, bx} at all. */
    goto repeat;
}

