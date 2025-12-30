#include "markdownPrinter.h"
#include <stdio.h>
int main(int argc, char *argv[]){
    int i = 0;
	int retval = -1;

	if (argc < 2) {
		fprintf(stderr, "mdcat: 缺失文件");
		fprintf(stderr, "usage: mdcat FILEs ...\n");
		exit(1);
	}

	for (i = 1; i < argc; i++) {
		retval = mdcat_worker(argv[i]);
	}

	return retval;
}