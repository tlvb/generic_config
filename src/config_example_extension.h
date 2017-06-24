#pragma once
#include "config_core.h"
#include <inttypes.h>
#include <stdio.h>

typedef struct { /*{{{*/
	size_t n;
	char **s;
} stringlist; /*}}}*/

bool read_stringlist(void *sl, FILE *fd);
void write_stringlist(FILE *fd, const void *sl, size_t indentlevel);
void free_stringlist(void *sl);

