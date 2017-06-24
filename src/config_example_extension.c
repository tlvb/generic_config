#include "config_example_extension.h"
#include <stdlib.h>
#include <assert.h>

bool read_stringlist(void *sl, FILE *fd) { /*{{{*/
	stringlist *tm = sl;
	if (1 != fscanf(fd, " %zu", &tm->n)) { return false; }
	tm->s = malloc(tm->n*sizeof(char*));
	assert(tm->s != NULL);
	for (size_t i=0; i<tm->n; ++i) {
		if (1 != fscanf(fd, " %m[^\n]", tm->s+i)) { return false; }
	}
	return true;
} /*}}}*/
void write_stringlist(FILE *fd, const void *sl, size_t indentlevel) { /*{{{*/
	const stringlist *tm = (const stringlist *)sl;
	fprintf(fd, "%zu\n", tm->n);
	for (size_t i=0; i<tm->n; ++i) {
		indent(fd, indentlevel+1);
		fprintf(fd, "%s\n", tm->s[i]);
	}
} /*}}}*/
void free_stringlist(void *sl) { /*{{{*/
	stringlist *tm = sl;
	for (size_t i=0; i<tm->n; ++i) {
		free(tm->s[i]);
	}
	free(tm->s);
} /*}}}*/

