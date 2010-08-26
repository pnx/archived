
#include <stdio.h>
#include "../src/arch/db.h"

int main(int argc, char *argv[]) {
	
	printf("Run!\n");
	
	arch_db_init("localhost", "warez", "elebobo", "filesystem", "filesystem");
	
	sleep(10);
	
	arch_db_insert("/this/is/my/path/to/", "myfile", 0);
	
	arch_db_delete("/this/is/my/path/to/", "myfile");
	
	arch_db_close();
	
	return 0;
}
