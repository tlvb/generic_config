#include "config_core.h"
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

#define CFG_FAIL_IF(expr, ...) if (expr) { fprintf(stderr, "(EE) CONFIG_CORE: " __VA_ARGS__); return false; }

static void _deflate_config(FILE *fd, const void *cfg, const config_mapping *map, size_t indentlevel);

void ignore_until_eol(FILE *f) {
	for (;;) {
		int c = fgetc(f);
		if (c < 0 || c == '\n') { return; }
	}
}
void ignore_comment(FILE *f) { /*{{{*/
	int t = fgetc(f);
	while (isspace(t)) {
		t = fgetc(f);
	}
	if (t == '#') {
		ignore_until_eol(f);
	}
	else {
		ungetc(t, f);
	}
} /*}}}*/
void indent(FILE *f, size_t indentlevel) {
	for (size_t i=0; i<indentlevel; ++i) {
		fputc('\t', f);
	}
}
bool inflate_config(void *cfg, const config_mapping *map, FILE* fd) { /*{{{*/
	for (;;) {
next:		ignore_comment(fd);
		char key[128];
		char tmp[2];
		key[0] = '\0';
		int kret = 0;
		kret = fscanf(fd, " %2[}]", (char*)&tmp);
		if (kret == 1) { return true; }
		kret = fscanf(fd, " %128[^= ] %2[=]", (char*)&key, (char*)&tmp);
		if (kret == EOF) { return true; }
		CFG_FAIL_IF(kret != 2, "parse error: an expected key was not followed by an expected equals sign or opening bracket (%d, %s)\n", kret, key);
		const config_mapping *nbq = map;
		while (nbq->field_key != NULL) {
			if (!strcmp(nbq->field_key, key)) {
				bool vret = false;
				switch(nbq->field_type) {
					case 's':
						vret = 1 == fscanf(fd, " %m[^\n]",  (char**)reconstruct_ptr(cfg, nbq->field_offset));
						break;
					case 'i':
						vret = 1 == fscanf(fd, " %" CFG_SCNi, (CFG_INT*)reconstruct_ptr(cfg, nbq->field_offset));
						break;
					case 'u':
						vret = 1 == fscanf(fd, " %" CFG_SCNu,  (CFG_UINT*)reconstruct_ptr(cfg, nbq->field_offset));
						break;
					case 'b':
						vret = 1 == fscanf(fd, " %2[yYnN]", (char*)&tmp);
						*(bool*)reconstruct_ptr(cfg, nbq->field_offset) = tmp[0] == 'y' || tmp[0] == 'Y';
						ignore_until_eol(fd);
						break;
					case '*':
						vret = nbq->op.r(reconstruct_ptr(cfg, nbq->field_offset), fd);
						break;
					case '+':
						ignore_until_eol(fd);
						vret = inflate_config(reconstruct_ptr(cfg, nbq->field_offset), nbq->sm, fd);
						break;
					default:
						CFG_FAIL_IF(true, "config type character 0x%x=%c for key \"%s\" not recognized\n", (unsigned int)nbq->field_type, nbq->field_type, key);
						break;
				}
				CFG_FAIL_IF(!vret, "parse error: failed to read value for key \"%s\"\n", key);
				goto next;
			}
			++nbq;
		}
		// we only ever exit the above while loop normally if we fail to find a matching key
		fprintf(stderr, "config file contains invalid key \"%s\"\n", key);
		fprintf(stderr, "valid keys are:\n");
		while (map->field_key != NULL) {
			fprintf(stderr, "%s\n", map->field_key);
		}
		return false;
	}
} /*}}}*/
void deflate_config(FILE *fd, const void *cfg, const config_mapping *map) { /*{{{*/
	_deflate_config(fd, cfg, map, 0);
}
void _deflate_config(FILE *fd, const void *cfg, const config_mapping *map, size_t indentlevel) { /*{{{*/
	while (map->field_key != NULL) {
		indent(fd, indentlevel);
		fprintf(fd, "%s = ", map->field_key);
		switch (map->field_type) {
			case 's':
				fprintf(fd, "%s\n", *((const char*const*)reconstruct_cptr(cfg, map->field_offset)));
				break;
			case 'i':
				fprintf(fd, "%" CFG_PRIi "\n", *((const CFG_INT*)reconstruct_cptr(cfg, map->field_offset)));
				break;
			case 'u':
				fprintf(fd, "%" CFG_PRIu "\n", *((const CFG_UINT*)reconstruct_cptr(cfg, map->field_offset)));
				break;
			case 'b':
				if (*((const bool*)reconstruct_cptr(cfg, map->field_offset))) {
					fprintf(fd, "yes\n");
				}
				else {
					fprintf(fd, "no\n");
				}
				break;
			case '*':
				map->op.w(fd, reconstruct_cptr(cfg, map->field_offset), indentlevel);
				break;
			case '+':
				fprintf(fd, "{\n");
				_deflate_config(fd, reconstruct_cptr(cfg, map->field_offset), map->sm, indentlevel+1);
				fprintf(fd, "}\n");
				break;
			default:
				break;
		}
		++map;
	}
} /*}}}*/
void free_config_contents(void *cfg, const config_mapping *map) { /*{{{*/
	while (map->field_key != NULL) {
		switch (map->field_type) {
			case 's':
				free(*((char**)reconstruct_ptr(cfg, map->field_offset)));
				break;
			case '*':
				map->op.f(reconstruct_ptr(cfg, map->field_offset));
				break;
			case '+':
				free_config_contents(reconstruct_ptr(cfg, map->field_offset), map->sm);
				break;
			default:
				break;
		}
		++map;
	}
} /*}}}*/
