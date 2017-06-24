#pragma once
#include <stdbool.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>

#ifndef CFG_INTEGER_BITS
#define CFG_INTEGER_BITS 32
#endif

#if   CFG_INTEGER_BITS == 8
#define CFG_INT  int8_t
#define CFG_UINT uint8_t
#define CFG_SCNu SCNu8
#define CFG_SCNi SCNi8
#define CFG_PRIu PRIu8
#define CFG_PRIi PRIi8
#elif CFG_INTEGER_BITS == 16
#define CFG_INT  int16_t
#define CFG_UINT uint16_t
#define CFG_SCNu SCNu16
#define CFG_SCNi SCNi16
#define CFG_PRIu PRIu16
#define CFG_PRIi PRIi16
#elif CFG_INTEGER_BITS == 32
#define CFG_INT  int32_t
#define CFG_UINT uint32_t
#define CFG_SCNu SCNu32
#define CFG_SCNi SCNi32
#define CFG_PRIu PRIu32
#define CFG_PRIi PRIi32
#elif CFG_INTEGER_BITS == 64
#define CFG_INT  int64_t
#define CFG_UINT uint64_t
#define CFG_SCNu SCNu64
#define CFG_SCNi SCNi64
#define CFG_PRIu PRIu64
#define CFG_PRIi PRIi64
#else
#ifndef CFG_INT
#warn "No default integer types provided and none supplied"
#endif
#endif

#define reconstruct_ptr(p, o) ((void*)(((uint8_t *)(p))+(o)))
#define reconstruct_cptr(p, o) ((const void*)(((const uint8_t *)(p))+(o)))

typedef bool (*read_op)(void*, FILE*);
typedef void (*write_op)(FILE*, const void*, size_t);
typedef void (*free_op)(void*);

typedef struct config_mapping { /*{{{*/
	char *field_key;
	char field_type;
	size_t field_offset;
	union {
		const struct config_mapping *sm;
		const struct {
			read_op r;
			write_op w;
			free_op f;
		} op;
	};
} config_mapping; /*}}}*/

void ignore_comment(FILE *f);
void ignore_until_eol(FILE *f);
void indent(FILE *f, size_t indentlevel);
bool inflate_config(void *cfg, const config_mapping *map, FILE *fd);
void deflate_config(FILE *fd, const void *cfg, const config_mapping *map);
void free_config_contents(void *cfg, const config_mapping *map);


