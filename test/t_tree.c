
#include <stdio.h>
#include "../src/fs/tree.h"

int main(int argc, char *argv[]) {
	
	struct entry *ent;
	struct tree  *tree;
	unsigned int c_ent = 0;
	
	if (argc < 2)
		return 1;
	
	tree = tree_new(argv[1]);
	
	if (tree == NULL) {
		printf("error\n");
		return 1;
	}
	
	for(;;) {
		
		ent = tree_next_ent(tree);
		
		if (ent == NULL)
			break;
		
		printf("%s%s\n", ent->base, ent->name);
		
		c_ent++;
	}
	
	printf("%i Entries\n", c_ent);

	return 0;
}
