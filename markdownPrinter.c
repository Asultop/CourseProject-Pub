#include "markdownPrinter.h"

/* Define globals declared extern in header */
mdcat_t mdcat = { .fmt = DO_RESET };
stack_t stack;

/* Helper: append to dstline at dstindx, ensure null-termination */
static void append_str(char **dstline, int *dstindx, const char *s) {
	int sl = (int)strlen(s);
	memcpy(*dstline + *dstindx, s, sl);
	*dstindx += sl;
	(*dstline)[*dstindx] = '\0';
}

/* Render simple math between $ ... $ supporting A_i -> A_i (use Unicode subscript digits when possible) */
char *mdcat_render_math(char **dstline, int *dstindx, char *lineptr, int *ip) {
	int i = *ip; /* points at the opening $ */
	int start = ++i;
	char buf[256];
	int bi = 0;

	/* find closing $ */
	while (lineptr[i] != '\0' && lineptr[i] != '$' && bi < (int)sizeof(buf)-1) {
		/* handle \dots */
		if (lineptr[i] == '\\' && strncmp(&lineptr[i], "\\dots", 5) == 0) {
			buf[bi++] = '\xE2'; /* … (UTF-8) 0xE2 0x80 0xA6 */
			buf[bi++] = '\x80';
			buf[bi++] = '\xA6';
			i += 5;
			continue;
		}

		/* handle underscore for simple subscript like A_i or x_12 */
		if (lineptr[i] == '_' && lineptr[i+1] != '\0') {
			i++;
			/* collect subscript token */
			int j = i;
			char sub[64]; int sj = 0;
			if (lineptr[j] == '{') {
				j++;
				while (lineptr[j] != '\0' && lineptr[j] != '}' && sj < (int)sizeof(sub)-1) {
					sub[sj++] = lineptr[j++];
				}
				if (lineptr[j] == '}') j++;
			} else {
				while ((lineptr[j] >= '0' && lineptr[j] <= '9') || (lineptr[j] >= 'a' && lineptr[j] <= 'z') || (lineptr[j] >= 'A' && lineptr[j] <= 'Z') ) {
					sub[sj++] = lineptr[j++];
				}
			}
			sub[sj] = '\0';

			/* convert digits to Unicode subscript digits when possible */
			for (int k = 0; k < sj; k++) {
				char c = sub[k];
				if (c >= '0' && c <= '9') {
					/* map 0-9 to U+2080..U+2089 */
					unsigned int code = 0x2080 + (c - '0');
					/* encode UTF-8 */
					buf[bi++] = (char)(0xE0 | ((code >> 12) & 0x0F));
					buf[bi++] = (char)(0x80 | ((code >> 6) & 0x3F));
					buf[bi++] = (char)(0x80 | (code & 0x3F));
				} else {
					/* fallback: use underscore plus char */
					buf[bi++] = c;
				}
			}
			i = j;
			continue;
		}

		buf[bi++] = lineptr[i++];
	}

	buf[bi] = '\0';

	/* append rendered math (simple) to dstline (no surrounding $) */
	append_str(dstline, dstindx, buf);

	if (lineptr[i] == '$') {
		*ip = i; /* point to closing $ */
	} else {
		*ip = i - 1;
	}

	return *dstline;
}

int mdcat_print_line(char *line){
	char *ptr = line;
	size_t delay = 3000;

	while(*ptr != '\0') {
		printf("%c", *ptr++);
		usleep((useconds_t)delay);
		fflush(stdout);
	}

	printf("\n");

	return 0;
}

bool is_header(const char *str){
	bool retval = false;

	do {
		if ((str[0] == '#') && (str[1] == ' '))  {
			retval = true;
		} else if ((str[0] == '#') && (str[1] == '#') && (str[2] == ' ')) {
			retval = true;
		} else if ((str[0] == '#') && (str[1] == '#') && (str[2] == '#') && (str[3] == ' ')) {
			retval = true;
		} else {
			retval = false;
		}
	} while(0);

	return retval;
}

char *mdcat_render_header(char **dstline, char *str){
	char *lineptr = NULL;

	do {
		assert(str != NULL);
		assert(dstline != NULL);
		assert(*dstline != NULL);

		if ((str[0] == '#') && (str[1] == ' '))  {
			lineptr = &str[2];
			sprintf(*dstline, ANSI_BOLD_BLUE "%s" ANSI_FRMT_RESET, lineptr);
		} else if ((str[0] == '#') && (str[1] == '#') && (str[2] == ' ')) {
			lineptr = &str[3];
			sprintf(*dstline, ANSI_BOLD_YELLOW "%s" ANSI_FRMT_RESET, lineptr);
		} else if ((str[0] == '#') && (str[1] == '#') && (str[2] == '#') && (str[3] == ' ')) {
			lineptr = &str[4];
			sprintf(*dstline, ANSI_BOLD_MAGENTA "%s" ANSI_FRMT_RESET, lineptr);
		} else {
			lineptr = NULL;
		}
	} while(0);

	return lineptr;
}

char *mdcat_render_text(char **dstline, int *dstindx, int md_op, char *fmt){
	int retval = -1;

	do {
			assert(dstindx  != NULL);
			assert(dstline  != NULL);
			assert(*dstline != NULL);

			/* Get current format from the stack top */
			retval = peek(&stack);
			if (retval == md_op) {
					mdcat.fmt = DO_RESET;
					strcat(*dstline, ANSI_FRMT_RESET);        /* 1: Reset the current text format */
					*dstindx += (int)strlen(ANSI_FRMT_RESET); /* 2: Adjust the dstline pointer to the end of format specifier to load next text */
					retval = pop(&stack);                     /* 3: Popout the handled format from the stack */
					assert(retval == md_op);
			} else {
					mdcat.fmt = md_op;
					strcat(*dstline, fmt);        /* 1: Insert the text format specifier */
					*dstindx += (int)strlen(fmt); /* 2: Adjust the dstline pointer to the end of format specifier to load next text */
					retval = push(&stack, md_op); /* 3: Push the current format to the stack */
					assert(retval == STACK_OK);
			}
	} while(0);

	return *dstline;
}

char *mdcat_render_list(char *dstline, char *str){
	int        i = 0;
	int  dstindx = 0;
	char *retval = NULL;
	bool applied = false;
	char *bullet = "•"; /* "➨" */
	int     blen = (int)strlen(bullet);

	do {
		assert(str != NULL);
		assert(dstline != NULL);

		if (mdcat.fmt == DO_CODEBLOCK) {
			retval = NULL;
			break;
		}

		while (str[i] == ' ' || str[i] == '\t') {
			i++;
		}

		if (((str[i] == '-') && (str[i + 1] == ' ')) ||
		    ((str[i] == '*') && (str[i + 1] == ' '))) {
			applied = false;
			for (i = 0, dstindx = 0; str[i] != '\0'; i++, dstindx++) {
				if ((applied != true) && ((str[i] == '-') || (str[i] == '*'))) {
					i++;
					strcat(dstline, bullet);
					dstindx += blen;
					applied = true;
				}
				dstline[dstindx] = str[i];
			}
		} else {
			retval = NULL;
			break;
		}

		retval = dstline;
	} while(0);

	return retval;
}

int mdcat_render_line(char *str, size_t len){
	int         i =  0;
	int    retval = -1;
	int   dstindx =  0;
	char *dstline = NULL; /* Holds the final ASCII formatted line */
	char *lineptr = NULL; /* Holds the starting of next line to render */


	do {
		lineptr = str;
		dstline = calloc(len * 2, sizeof(char)); /* @len: double the space of source line length to adapt the format specifiers */
		assert(dstline != NULL);

		/* Render #ed header line format */
		if (mdcat_render_header(&dstline, str) != NULL) {
			goto PRINT_OUTPUT;
			retval = MDCAT_OK;
			break;
		}

		/* Render list format */
		if (mdcat_render_list(dstline, str) != NULL) {
			goto PRINT_OUTPUT;
			retval = MDCAT_OK;
			break;
		}

		/* Render remaining formats */
		for (i = 0, dstindx = 0; lineptr[i] != '\0'; ) {
			/* handle inline math $...$ */
			if (lineptr[i] == '$') {
				mdcat_render_math(&dstline, &dstindx, lineptr, &i);
				/* move past the closing $ if present */
				if (lineptr[i] == '$') i++;
				continue;
			}

			/* handle \dots outside math */
			if (lineptr[i] == '\\' && strncmp(&lineptr[i], "\\dots", 5) == 0) {
				char ell[] = "\xE2\x80\xA6";
				/* convert escape-sequence string into actual bytes */
				char ellutf[4] = { (char)0xE2, (char)0x80, (char)0xA6, '\0' };
				append_str(&dstline, &dstindx, ellutf);
				i += 5;
				continue;
			}

			switch (lineptr[i]) {
				case '*':
					if (mdcat.fmt == DO_CODEBLOCK) {
						break;
					}

					if (lineptr[i + 1] == '*') {
						i += 2;
						/* Render Bold format */
						mdcat_render_text(&dstline, &dstindx, DO_BOLD, ANSI_FRMT_BOLD);
					} else {
						i += 1;
						/* Render Italic format */
						mdcat_render_text(&dstline, &dstindx, DO_ITALICS, ANSI_FRMT_ITALICS);
					}

					break;
				case '_': /* Render Underline format */
					if (mdcat.fmt == DO_CODEBLOCK) {
						break;
					}

					if ((lineptr[i + 1] == '_') && (lineptr[i + 2] == '_')) {
						i += 3;
						mdcat_render_text(&dstline, &dstindx, DO_UNDERLINE, ANSI_FRMT_UNDERLINE);
					}

					break;

				case '`': /* Render code snippet */
					if ((lineptr[i + 1] == '`') && (lineptr[i + 2] == '`')) {
						i += 3;
					} else if (lineptr[i + 1] == '`') {
						i += 2;
					} else {
						i += 1;
					}

					mdcat_render_text(&dstline, &dstindx, DO_CODEBLOCK, ANSI_COLOR_MAGENTA);
					break;
				default:
					break;
			}

			/* copy current character to output */
			dstline[dstindx] = lineptr[i];
			dstindx++;
			dstline[dstindx] = '\0';
			i++;
			fflush(stdout);
		}

PRINT_OUTPUT:
		mdcat_print_line(dstline);
		free(dstline);
	} while(0);

	return retval;
}

int mdcat_worker(const char *file){
	int retval = -1;
	size_t len =  0;
	FILE   *fp = NULL;
	char *linebuff = NULL;

	logit("%s", file);

	do {
		retval = initialize_stack(&stack);
		if (retval != STACK_OK) {
			fprintf(stderr, "failed to init stack");
			break;
		}

		fp = fopen(file, "r");
		if (fp == NULL) {
			fprintf(stderr, "mdcat: cannot open '%s' (%s)\n", file, strerror(errno));
			break;
		}

		/*
		  Getline dynamically allocates memory for the line, and it automatically resizes the buffer if needed.
		  The function returns the number of characters read, and -1 if there's an error or the end of the file.
		*/
		while (getline(&linebuff, &len, fp) != -1) {
			linebuff[strlen(linebuff) - 1] = '\0';
			mdcat_render_line(linebuff, len - 1);
		}

		//display_stack(&stack);
		free(linebuff);
		fclose(fp);
	} while(0);

	logit("retval: %d", retval);

	return retval;

}