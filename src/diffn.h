
#include <sys/stat.h>
#include <string>
#include <vector>

#include "difdef.h"

struct FileInfo {
    std::string name;
    struct stat stat;
};

extern std::vector<std::string> g_MacroNames;

void verify_properly_nested_directives(const Difdef::Diff &diff, const FileInfo files[]);
bool matches_pp_directive(const std::string &s, const char *directive);
bool matches_if_directive(const std::string &s);
void do_print_using_ifdefs(const Difdef::Diff &diff);
void do_print_unified_diff(const Difdef::Diff &diff, const FileInfo files[], size_t lines_of_context);
void do_error(const char *fmt, ...);

