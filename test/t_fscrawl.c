
#include <stdio.h>
#include "../src/fscrawl.h"

int main(int argc, char *argv[]) {
	
	fscrawl_t crawl;
	unsigned int c_ent = 0;
	
	if (argc < 2)
		return 1;
	
	crawl = fsc_open(argv[1]);
	
	if (crawl == NULL) {
		printf("Invalid path\n");
		return 1;
	}
	
	for(;;) {
		
		fs_entry *ent = fsc_read(crawl);

        if (!ent)
			break;
		
		printf("%s%s", ent->base, ent->name);

        if (ent->dir)
            printf("/\n");
        else
            printf("\n");
		
		c_ent++;
	}
	
	printf("%i Entries\n", c_ent);

	return 0;
}
