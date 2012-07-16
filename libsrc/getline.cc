
/*
   The |getline| library by Arthur O'Dwyer.
   Public domain.
*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

#include "getline.h"


static void try_resize(char **p, size_t *cap);


/*
   The |fgetline_notrim| function reads characters from |stream| and
   stores them into a dynamically allocated buffer pointed to by |*s|.
   (The old value of |*s| is ignored by this function.)
   Reading stops after an |EOF| or a newline. If a newline is read,
   it is stored into the buffer.

   |fgetline_notrim| returns |p| on success; and |NULL| on I/O error,
   or when end-of-file occurs while no characters have been read,
   or when the line length exceeds |(size_t)-1|,
   or when a call to |malloc| returns |NULL|. In all four cases,
   the characters which have been read so far, if any, still
   reside in the dynamically allocated buffer |*p|.
*/
char *fgetline_notrim(char **p, FILE *stream)
{
    size_t cap = 0;
    size_t len = 0;
    char *rc;

    *p = NULL;

    while (1) {
        try_resize(p, &cap);
        if (cap-1 <= len) {
            /* |try_resize| failed */
            return NULL;
        }
        rc = fgets(*p+len, cap-len, stream);
        if (rc == NULL) {
            /*
               EOF or input error. In either case, once |NULL| has
               been returned, the contents of the buffer are unusable.
            */
            (*p)[len] = '\0';
            if (feof(stream) && len > 0)
              return *p;
            else return NULL;
        }
        else if (strchr(*p+len, '\n') != NULL) {
            /* a newline has been read */
            return *p;
        }
        else {
            /* we must continue reading */
            len += strlen(*p+len);
            if (feof(stream))
              return *p;
        }
    }
}


/*
   The |try_resize| function tries to resize |*p| so it can hold more
   data. It will always yield a valid, consistent |*p| and |*cap| ---
   |*cap| will never decrease, and no data will be lost from |*p|.
   But if a call to |realloc| fails, |*cap| will be unchanged.
   One important thing to notice: We must never increase |*cap| by
   more than |INT_MAX|, since the second parameter to |fgets| is of
   type |int|.
*/
static void try_resize(char **p, size_t *cap)
{
    /*
       We aren't expecting any really long lines, here. But if the
       current line has exceeded 500 characters, there's probably
       something special going on (like an attempted buffer overflow
       attack), and we'll start increasing the buffer capacity
       geometrically.
    */
    size_t newcap = (*cap < 500)? (*cap + 16):
                    (*cap/2 < INT_MAX)? (*cap + *cap/2):
                    (*cap + INT_MAX);
    char *newp;
    if (newcap < *cap) {
        /* The line length has exceeded |(size_t)-1|. Wow! */
        if (*cap == (size_t)-1) return;
        else newcap = (size_t)-1;
    }

    newp = (char *)realloc(*p, newcap);
    /* Maybe we can't get that much memory. Try smaller chunks. */
    while (newp == NULL) {
        newcap = *cap + (newcap - *cap)/2;
        if (newcap == *cap) break;
        newp = (char *)realloc(*p, newcap);
    }

    if (newp != NULL)
      *p = newp;

    /* At this point, |*p| hasn't lost any data, and |newcap| is valid. */
    *cap = newcap;
    return;
}

/* Read and return a single line from the given stream. This is equivalent
 * to std::getline(std::istream &), except that it works with C I/O. */
bool getline(FILE *stream, std::string &line)
{
    char *pline = NULL;
    char *rc = fgetline_notrim(&pline, stream);
    if (rc == NULL && (pline == NULL || pline[0] == '\0')) {
        /* Out of memory, I/O error, or end-of-file.
         * No characters were read. */
        free(pline);
        return false;
    }
    /* Trim the terminating newline. */
    rc = strchr(pline, '\n');
    if (rc != NULL) {
        *rc = '\0';
    }
    line = pline;
    free(pline);
    return true;
}
