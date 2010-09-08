
#include <stdio.h>
#include <string.h>
#include <inotifytools/inotifytools.h>
#include <inotifytools/inotify.h>
// 		      (IN_MOVE | IN_CREATE | IN_DELETE | IN_ONLYDIR)
#define WMASK (IN_MOVE | IN_MOVE_SELF | IN_CREATE | IN_DELETE | IN_ONLYDIR)

int main(int argc, char **argv) {
	
	struct inotify_event *ev;
	
	if (argc < 2)
		return 1;
	
	if ( !inotifytools_initialize()
		 || !inotifytools_watch_recursively(argv[1], WMASK) ) {
			fprintf(stderr, "%s\n", strerror(inotifytools_error()) );
	        return 1;
	}
	
    for(;;) {
    	
    	ev = inotifytools_next_event(-1);
    	
    	printf("%u: ", ev->wd);
        inotifytools_printf(ev, "%w:%f %e\n");
    }

	return 0;
}
