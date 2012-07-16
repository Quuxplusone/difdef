
/*
   The |getline| library by Arthur O'Dwyer.
   Public domain.
*/


#ifndef H_GETLINE
 #define H_GETLINE

#include <stdio.h>
#include <string>

char *fgetline_notrim(char **p, FILE *stream);

bool getline(FILE *stream, std::string &line);

#endif
