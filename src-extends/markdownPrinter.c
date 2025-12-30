#include "markdownPrinter.h"
#include <ctype.h>
#include <string.h>
#include <stdbool.h>


mdcat_t mdcat = { .fmt = DO_RESET };
stack_t stack;

static useconds_t mdcat_delay_us = 0;

/* Helper: append to dstline at dstindx, ensure null-termination */
static void append_str(char **dstline, int *dstindx, const char *s) {
	int sl = (int)strlen(s);
	memcpy(*dstline + *dstindx, s, sl);
	*dstindx += sl;
	(*dstline)[*dstindx] = '\0';
}

/* Append a Unicode codepoint as UTF-8 to a byte buffer */
static bool append_codepoint_utf8(char *buf, int *bi, unsigned int code, int bufsize) {
	if (*bi >= bufsize - 1) return false;
	if (code <= 0x7F) {
		if (*bi + 1 >= bufsize) return false;
		buf[(*bi)++] = (char)code;
	} else if (code <= 0x7FF) {
		if (*bi + 2 >= bufsize) return false;
		buf[(*bi)++] = (char)(0xC0 | ((code >> 6) & 0x1F));
		buf[(*bi)++] = (char)(0x80 | (code & 0x3F));
	} else if (code <= 0xFFFF) {
		if (*bi + 3 >= bufsize) return false;
		buf[(*bi)++] = (char)(0xE0 | ((code >> 12) & 0x0F));
		buf[(*bi)++] = (char)(0x80 | ((code >> 6) & 0x3F));
		buf[(*bi)++] = (char)(0x80 | (code & 0x3F));
	} else {
		if (*bi + 4 >= bufsize) return false;
		buf[(*bi)++] = (char)(0xF0 | ((code >> 18) & 0x07));
		buf[(*bi)++] = (char)(0x80 | ((code >> 12) & 0x3F));
		buf[(*bi)++] = (char)(0x80 | ((code >> 6) & 0x3F));
		buf[(*bi)++] = (char)(0x80 | (code & 0x3F));
	}
	return true;
}

/* Map common LaTeX commands (after '\\') to Unicode codepoints. Returns true if mapped and sets consumed length and code. */
static bool map_latex_cmd(const char *s, int *consumed, unsigned int *out_code) {
	/* s points at first char after backslash */
	struct { const char *name; unsigned int code; } table[] = {
		{"alpha", 0x03B1}, {"beta", 0x03B2}, {"gamma", 0x03B3}, {"delta", 0x03B4},
		{"epsilon", 0x03B5}, {"zeta", 0x03B6}, {"eta", 0x03B7}, {"theta", 0x03B8},
		{"iota", 0x03B9}, {"kappa", 0x03BA}, {"lambda", 0x03BB}, {"mu", 0x03BC},
		{"nu", 0x03BD}, {"xi", 0x03BE}, {"omicron", 0x03BF}, {"pi", 0x03C0},
		{"rho", 0x03C1}, {"sigma", 0x03C3}, {"tau", 0x03C4}, {"upsilon", 0x03C5},
		{"phi", 0x03C6}, {"chi", 0x03C7}, {"psi", 0x03C8}, {"omega", 0x03C9},
		{"Gamma", 0x0393}, {"Delta", 0x0394}, {"Theta", 0x0398}, {"Lambda", 0x039B},
		{"Xi", 0x039E}, {"Pi", 0x03A0}, {"Sigma", 0x03A3}, {"Phi", 0x03A6},
		{"Omega", 0x03A9}, {NULL, 0}
	};

	for (int i = 0; table[i].name != NULL; i++) {
		int ln = (int)strlen(table[i].name);
		if (strncmp(s, table[i].name, ln) == 0) {
			*consumed = ln;
			*out_code = table[i].code;
			return true;
		}
	}

	return false;
}

/* Render simple math between $ ... $ supporting A_i -> A_i (use Unicode subscript digits when possible) */
char *mdcat_render_math(char **dstline, int *dstindx, char *lineptr, int *ip) {
	int i = *ip; /* points at the opening $ */
	i++; /* move past opening $ */
	int start = i;
	char buf[1024];
	int bi = 0;

	/* helper to append codepoint utf-8 to buf */
	auto_append_codepoint:
	;

	/* iterate tokens until closing $ */
	while (lineptr[i] != '\0' && lineptr[i] != '$' && bi < (int)sizeof(buf)-1) {
		/* handle backslash escapes: \dots or LaTeX commands like \alpha */
		if (lineptr[i] == '\\') {
			/* \dots special-case kept for compatibility */
			if (strncmp(&lineptr[i], "\\dots", 5) == 0) {
				if (bi + 3 < (int)sizeof(buf)-1) {
					buf[bi++] = (char)0xE2; buf[bi++] = (char)0x80; buf[bi++] = (char)0xA6;
				}
				i += 5;
				continue;
			}

			/* Map common LaTeX commands (e.g. \alpha) */
			int consumed = 0;
			unsigned int code = 0;
			if (map_latex_cmd(&lineptr[i+1], &consumed, &code)) {
				/* append mapped codepoint */
				append_codepoint_utf8(buf, &bi, code, (int)sizeof(buf));
				i += 1 + consumed;
				continue;
			}
			/* unknown escape: copy backslash as-is */
			buf[bi++] = lineptr[i++];
			continue;
		}

		/* skip spaces and copy them */
		if (lineptr[i] == ' ') {
			buf[bi++] = ' ';
			i++;
			continue;
		}

		/* read base token (letters/digits) */
		if ((lineptr[i] >= 'A' && lineptr[i] <= 'Z') || (lineptr[i] >= 'a' && lineptr[i] <= 'z') || (lineptr[i] >= '0' && lineptr[i] <= '9')) {
			int bstart = i;
			while ((lineptr[i] >= 'A' && lineptr[i] <= 'Z') || (lineptr[i] >= 'a' && lineptr[i] <= 'z') || (lineptr[i] >= '0' && lineptr[i] <= '9')) i++;
			int bend = i;
			/* check for subscript or superscript */
			if ((lineptr[i] == '_' || lineptr[i] == '^') && lineptr[i+1] != '\0') {
				char sym = lineptr[i]; /* '_' or '^' */
				/* capture subscript raw */
				int j = i+1;
				bool has_brace = false;
				if (lineptr[j] == '{') { has_brace = true; j++; }
				int sj = 0;
				char sub[128];
				while (lineptr[j] != '\0' && ((has_brace && lineptr[j] != '}') || (!has_brace && ((lineptr[j] >= '0' && lineptr[j] <= '9') || (lineptr[j] >= 'a' && lineptr[j] <= 'z') || (lineptr[j] >= 'A' && lineptr[j] <= 'Z') || (lineptr[j] == '+' || lineptr[j] == '-' || lineptr[j] == '=' || lineptr[j] == '(' || lineptr[j] == ')')))) && sj < (int)sizeof(sub)-1) {
					sub[sj++] = lineptr[j++];
				}
				sub[sj] = '\0';
				if (has_brace && lineptr[j] == '}') j++;

				/* try map every char in sub to sub/superscript codepoints */
				bool all_mapped = true;
				char tmp[256]; int tbi = 0;
				for (int k = 0; k < sj; k++) {
					char c = sub[k];
					if (sym == '_') {
						/* subscript mapping */
						if (c >= '0' && c <= '9') {
							unsigned int code = 0x2080 + (c - '0');
							/* 0x2080..209F are multi-byte in UTF-8 */
							append_codepoint_utf8(tmp, &tbi, code, (int)sizeof(tmp));
						} else {
							unsigned int code = 0;
							switch (c) {
								case 'a': code = 0x2090; break; case 'e': code = 0x2091; break; case 'o': code = 0x2092; break; case 'x': code = 0x2093; break;
								case 'h': code = 0x2095; break; case 'k': code = 0x2096; break; case 'l': code = 0x2097; break; case 'm': code = 0x2098; break;
								case 'n': code = 0x2099; break; case 'p': code = 0x209A; break; case 's': code = 0x209B; break; case 't': code = 0x209C; break;
								case 'i': code = 0x1D62; break; default: code = 0; break;
							}
							if (code != 0) append_codepoint_utf8(tmp, &tbi, code, (int)sizeof(tmp)); else { all_mapped = false; break; }
						}
					} else {
						/* superscript mapping */
						if (c >= '0' && c <= '9') {
							unsigned int code;
							switch (c) {
								case '0': code = 0x2070; break; case '1': code = 0x00B9; break; case '2': code = 0x00B2; break; case '3': code = 0x00B3; break;
								case '4': code = 0x2074; break; case '5': code = 0x2075; break; case '6': code = 0x2076; break; case '7': code = 0x2077; break;
								case '8': code = 0x2078; break; case '9': code = 0x2079; break; default: code = 0; break;
							}
							append_codepoint_utf8(tmp, &tbi, code, (int)sizeof(tmp));
						} else {
							unsigned int code = 0;
							switch (c) {
								case '+': code = 0x207A; break; case '-': code = 0x207B; break; case '=': code = 0x207C; break;
								case '(' : code = 0x207D; break; case ')' : code = 0x207E; break; case 'n': code = 0x207F; break;
								default: code = 0; break;
							}
							if (code != 0) append_codepoint_utf8(tmp, &tbi, code, (int)sizeof(tmp)); else { all_mapped = false; break; }
						}
					}
				}

				if (all_mapped) {
					/* append base */
					for (int p = bstart; p < bend && bi < (int)sizeof(buf)-1; p++) buf[bi++] = lineptr[p];
					/* append mapped sub/superscript */
					for (int p = 0; p < tbi && bi < (int)sizeof(buf)-1; p++) buf[bi++] = tmp[p];
				} else {
					/* fallback: output original text including underscore/caret/braces */
					for (int p = bstart; p < bend && bi < (int)sizeof(buf)-1; p++) buf[bi++] = lineptr[p];
					/* append marker and raw sub */
					if (bi < (int)sizeof(buf)-1) buf[bi++] = sym;
					if (has_brace && bi < (int)sizeof(buf)-1) buf[bi++] = '{';
					for (int p = 0; p < sj && bi < (int)sizeof(buf)-1; p++) buf[bi++] = sub[p];
					if (has_brace && bi < (int)sizeof(buf)-1) buf[bi++] = '}';
				}

				i = j; /* advance past subscript/superscript */
				continue;
			} else {
				/* no sub/sup - append base */
				for (int p = bstart; p < bend && bi < (int)sizeof(buf)-1; p++) buf[bi++] = lineptr[p];
				continue;
			}
		}

		/* otherwise copy single char */
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
	if (mdcat_delay_us == 0) {
		puts(line);
		return 0;
	}

	char *ptr = line;
	while (*ptr != '\0') {
		putchar(*ptr++);
		usleep(mdcat_delay_us);
	}
	putchar('\n');
	fflush(stdout);
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

		/* Only detect header level and return pointer to header body.
		   Actual rendering (with inline code handling) is done in mdcat_render_line. */
		if ((str[0] == '#') && (str[1] == ' '))  {
			lineptr = &str[2];
		} else if ((str[0] == '#') && (str[1] == '#') && (str[2] == ' ')) {
			lineptr = &str[3];
		} else if ((str[0] == '#') && (str[1] == '#') && (str[2] == '#') && (str[3] == ' ')) {
			lineptr = &str[4];
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
				    append_str(dstline, dstindx, ANSI_FRMT_RESET);        /* Reset format */
				    retval = pop(&stack);
				    assert(retval == md_op);
			    } else {
				    mdcat.fmt = md_op;
				    append_str(dstline, dstindx, fmt);
				    retval = push(&stack, md_op);
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

		/* skip leading whitespace */
		while (str[i] == ' ' || str[i] == '\t') i++;

		/* detect list marker */
		if (((str[i] == '-') && (str[i + 1] == ' ')) || ((str[i] == '*') && (str[i + 1] == ' '))) {
			/* place bullet followed by a space for readability */
			append_str(&dstline, &dstindx, bullet);
			dstline[dstindx++] = ' ';
			dstline[dstindx] = '\0';
			/* advance past marker and following space */
			i += 2;

			/* now process remaining content but allow inline math, code spans and \dots */
			for (; str[i] != '\0' && dstindx < (int)strlen(dstline) + 1024; ) {
				/* If currently inside inline code block, copy verbatim until closing backticks */
				if (mdcat.fmt == DO_CODEBLOCK) {
					if (str[i] == '`') {
						int bt = 1;
						if (str[i+1] == '`') { bt++; if (str[i+2] == '`') bt++; }
						i += bt; /* skip closing backticks */
						mdcat_render_text(&dstline, &dstindx, DO_CODEBLOCK, ANSI_COLOR_MAGENTA);
						continue;
					} else {
						dstline[dstindx++] = str[i++];
						dstline[dstindx] = '\0';
						continue;
					}
				}

				/* handle opening backticks: start inline code span */
				if (str[i] == '`') {
					int bt = 1;
					if (str[i+1] == '`') { bt++; if (str[i+2] == '`') bt++; }
					i += bt; /* skip opening backticks */
					mdcat_render_text(&dstline, &dstindx, DO_CODEBLOCK, ANSI_COLOR_MAGENTA);
					continue;
				}

				/* handle math $...$ */
				if (str[i] == '$') {
					mdcat_render_math(&dstline, &dstindx, (char *)str, &i);
					if (str[i] == '$') i++; /* skip closing $ if present */
					continue;
				}

				/* handle \dots */
				if (str[i] == '\\' && strncmp(&str[i], "\\dots", 5) == 0) {
					char ellutf[4] = { (char)0xE2, (char)0x80, (char)0xA6, '\0' };
					append_str(&dstline, &dstindx, ellutf);
					i += 5;
					continue;
				}

				/* beautify commas inside list: if comma not followed by space, insert one */
				if (str[i] == ',') {
					dstline[dstindx++] = ',';
					if (str[i+1] != ' ') {
						dstline[dstindx++] = ' ';
					}
					i++;
					dstline[dstindx] = '\0';
					continue;
				}

				dstline[dstindx++] = str[i++];
				dstline[dstindx] = '\0';
			}

			retval = dstline;
		} else {
			retval = NULL;
			break;
		}
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

		/* Render #ed header line format with inline handling */
		{
			char *hbody = mdcat_render_header(&dstline, str);
			if (hbody != NULL) {
				/* determine header level (1..3) and choose color */
				const char *hcolor = ANSI_BOLD_BLUE;
				if (str[0] == '#' && str[1] == '#') hcolor = ANSI_BOLD_YELLOW;
				if (str[0] == '#' && str[1] == '#' && str[2] == '#') hcolor = ANSI_BOLD_MAGENTA;

				append_str(&dstline, &dstindx, hcolor);

				/* render header body with limited inline parsing (math, backticks, \dots) */
				for (int j = 0; hbody[j] != '\0' && dstindx < (int)strlen(dstline) + 1024; ) {
					/* handle inline code spans */
					if (mdcat.fmt == DO_CODEBLOCK) {
						if (hbody[j] == '`') {
							int bt = 1; if (hbody[j+1] == '`') { bt++; if (hbody[j+2] == '`') bt++; }
							j += bt;
							mdcat_render_text(&dstline, &dstindx, DO_CODEBLOCK, ANSI_COLOR_MAGENTA);
							continue;
						} else {
							dstline[dstindx++] = hbody[j++]; dstline[dstindx] = '\0'; continue;
						}
					}

					if (hbody[j] == '`') {
						int bt = 1; if (hbody[j+1] == '`') { bt++; if (hbody[j+2] == '`') bt++; }
						j += bt; mdcat_render_text(&dstline, &dstindx, DO_CODEBLOCK, ANSI_COLOR_MAGENTA); continue;
					}

					/* handle inline math */
					if (hbody[j] == '$') {
						mdcat_render_math(&dstline, &dstindx, hbody, &j);
						if (hbody[j] == '$') j++;
						continue;
					}

					/* handle \dots */
					if (hbody[j] == '\\' && strncmp(&hbody[j], "\\dots", 5) == 0) {
						char ellutf[4] = { (char)0xE2, (char)0x80, (char)0xA6, '\0' };
						append_str(&dstline, &dstindx, ellutf);
						j += 5; continue;
					}

					dstline[dstindx++] = hbody[j++]; dstline[dstindx] = '\0';
				}

				append_str(&dstline, &dstindx, ANSI_FRMT_RESET);
				goto PRINT_OUTPUT;
			}
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
		/* configure printing delay from env var MDCAT_DELAY_MS (milliseconds) */
		const char *env = getenv("MDCAT_DELAY_MS");
		if (env != NULL) {
			long ms = atol(env);
			if (ms < 0) ms = 0;
			mdcat_delay_us = (useconds_t)(ms * 1000);
		} else {
			/* default: no per-char delay for performance */
			mdcat_delay_us = 0;
		}

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