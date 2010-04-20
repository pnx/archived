
#ifndef INOTIFY_DEBUG
	#define INOTIFY_DEBUG
#endif

#include <stdio.h>
#include "../src/notify_db.h"

int main(int argc, char *argv[]) {
	
	// Keep me safe
	printf("Run!\n");
	
	// Connect
	notify_db_init("localhost", "warez", "elebobo", "filesystem", "filesystem");
	
	// Truncate
	//notify_db_truncate();
	
	sleep(10);
	
	// Insert
	notify_db_insert("/this/is/my/path/to/", "myfile", 0);
	
	// Delete
	//notify_db_delete("/this/is/my/path/to/", "myfile");
	
	// Close
	notify_db_close();
	
	// Kill me
	return 0;
}