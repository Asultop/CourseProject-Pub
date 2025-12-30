#ifndef MARKDOWNPRINTER_H
#define MARKDOWNPRINTER_H


#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "Def.h"
#include "stack.h"


#ifndef DEBUG
#define logit(fmt, ...) ((void) 0)
#else
#define logit(fmt, ...) (fprintf(stderr, "%17s:%-4d %17s: " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__))
#endif

enum {
	MDCAT_ERR = -1,
	MDCAT_OK  =  0,
};

enum fmts {
	DO_RESET     = 0xFFF0,
	DO_BOLD      = 0xFFF1,
	DO_CODEBLOCK = 0xFFF2,
	DO_ITALICS   = 0xFFF3,
	DO_LIST      = 0xFFF4,
	DO_UNDERLINE = 0xFFF5,
};

typedef struct mdcat_st {
	int fmt;
} mdcat_t;

extern mdcat_t mdcat;
extern stack_t stack;
int mdcat_print_line(char *line);
bool is_header(const char *str);
char *mdcat_render_header(char **dstline, char *str);
char *mdcat_render_text(char **dstline, int *dstindx, int md_op, char *fmt);
char *mdcat_render_list(char *dstline, char *str);
char *mdcat_render_math(char **dstline, int *dstindx, char *lineptr, int *ip);
int mdcat_render_line(char *str, size_t len);
int mdcat_worker(const char *file);

#endif // MARKDOWNPRINTER_H