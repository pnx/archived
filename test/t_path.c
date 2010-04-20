
#include <malloc.h>
#include <stdio.h>
#include "../src/common/path.h"

/* test data */

char string[8][128] = {
	"usr/include/",
	"/usr/src/linux",
	"/segment1/segment2/segment3/",
	"//double///tripple",
	"/stuff/with/ahell/lot/of/slashes/at/the/end/////////",
	"~/myhome/",
	"/",
	"///////////////////////////////////////////////////////////"
};

char split[8][2][64] = {
	{"usr/", "include/" },
	{"/usr/src/", "linux" },
	{"/segment1/segment2/", "segment3/" },
	{"//double//", "/tripple" },
	{"/stuff/with/ahell/lot/of/slashes/at/the/", "end/////////" },
	{"~/", "myhome/" },
	{"/mnt/cdrom", "keff" },
	{"//////////////////////////////", "/////////////////////////////" }
};

int main(int argc, char *argv[]) {

	int i;
	char *ptr = NULL;

	for(i=0; i < 8; i++) {
		//printf("adding: %s\n", string[i]);
		ptr = fmt_path(string[i], NULL, 0);
		printf("str %i is: %s\n", i, ptr);
		
		if (ptr != NULL) {
			free(ptr);
			ptr = NULL;
		}
	}
	
	for(i=0; i < 8; i++) {
		//printf("adding: %s%s\n", split[i][0], split[i][1]);
		ptr = fmt_path(split[i][0], split[i][1], 0);
		printf("str %i is: %s\n", i, ptr);
			
		if (ptr != NULL) {
			free(ptr);
			ptr = NULL;
		}
	}
	
	for(i=0; i < 8; i++) {
		//printf("adding: %s%s\n", split[i][0], split[i][1]);
		ptr = fmt_path(split[i][0], split[i][1], 1);
		printf("str %i is: %s\n", i, ptr);
				
		if (ptr != NULL) {
			free(ptr);
			ptr = NULL;
		}
	}
	
	if(argc < 2)
		return 0;
	else if(argc == 2) 
		ptr = fmt_path(argv[1], NULL, 0);
	else if(argc > 2) {
		ptr = fmt_path(argv[1], argv[2], 0);
	}
		
	printf("%s\n", ptr);
	
	if (ptr != NULL)
		free(ptr);

	return 0;
}
