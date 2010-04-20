
#include <unistd.h>
#include <stdio.h>
#include "../src/indexer.h"

int main() {
	
	indexer_register("/home/pnx/", "tmptree");
	indexer_register("/home/pnx/", "tmptree");
	indexer_register("/home/pnx/", "tmptree");
	
	while(indexer_pending()) {
		indexer_run(5);
		sleep(1);
	}
	
	printf("---\n");
	
	indexer_register("/home/pnx/", "tmptree");
	
	while(indexer_pending()) {
		indexer_run(5);
		sleep(1);
	}
	
	printf("done\n");
	
	return 0;
}
