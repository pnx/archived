
#include <malloc.h>
#include <stdio.h>
#include "../src/common/path.h"


int main(int argc, char *argv[]) {

	char fp[] = "/this/is/my/path/to/file.pdf/";
	char *d;
	printf("%s\n=>\n", fp);

	d = dirname(fp);

	printf("%s\n", d);

	return 0;
}
