#include <time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "arch/db.h"
#include "fs/notify.h"

/* only way to exit the application properly
   when in main loop is by signal */
static void clean_exit(int excode) {
	
	time_t t = time(NULL);
	
	notify_cleanup();
	arch_db_close();
	
	printf("\nprocess exit at: %s", ctime(&t));
	exit(excode);
}

/* signal handler */
static void sighandl(int sig) {
	
	switch(sig) {
	/* normal exit signals */
	case SIGTERM :
	case SIGKILL :
	case SIGQUIT :
	case SIGINT  :
		clean_exit(EXIT_SUCCESS);
	/* segmentation violation, let user now */
	case SIGSEGV :
		fprintf(stderr, "SEGFAULT: o no he didn't\n");
		clean_exit(EXIT_FAILURE);
	case SIGUSR1 :
	case SIGUSR2 :
		printf("notify stat:\n");
		notify_stat();
		/* don't know why, but everything goes bananas if we keep executing */
		clean_exit(EXIT_SUCCESS);
	}
}

void arch_loop() {

	notify_event *event;
	
	for(;;) {
		
		if (indexer_pending() && !notify_is_ready()) {
#ifdef __DEBUG__
        	printf("sched: running indexer\n");
#endif
            indexer_run(15);
            continue;
        }
        
#ifdef __DEBUG__
        printf("sched: notify block\n");  
#endif

		event = notify_read(-1);
		
		if (event == NULL)
			continue;
		
#ifdef __DEBUG__
		printf("-- EVENT --\n"
			   " TYPE: %s\n"
			   " DIR: %i\n"
			   " PATH: %s%s\n"
			   "---------------\n", notify_event_typetostr(event), event->dir, event->path, event->filename);
#endif
		
		switch(event->type) {
			
            case NOTIFY_MOVE_TO :
            	
                if (event->dir)
                    indexer_register(event->path, event->filename);
            case NOTIFY_CREATE :
            	arch_db_insert(event->path, event->filename, event->dir);
            	break;
            case NOTIFY_MOVE_FROM :
			case NOTIFY_DELETE :
				arch_db_delete(event->path, event->filename);
				break;
		}
		
#ifdef __DEBUG__
		notify_stat();
#endif
	}
}

int main(int argc, char **argv) {

	/* Validate arguments */
	if (argc != 7) {
		
		printf("Usage: %s <Root Directory> <Db user> <Db pass> <Db name>.\n"
			   "Root Directory - Path to indexroot. All subdirectories will be indexed.\n"
			   "Db host - Database host\n"
			   "Db user - Database user\n"
		       "Db pass - Database password\n"
		       "Db name - Database to use for indexing\n"
			   "Db tbl  - Database tablename", argv[0]);
		
		return EXIT_FAILURE;
	}

    /* setup signal handlers */
	signal(SIGTERM, sighandl);
	signal(SIGKILL, sighandl);
	signal(SIGQUIT, sighandl);
	signal(SIGINT, sighandl);
	signal(SIGSEGV, sighandl);
	signal(SIGUSR1, sighandl);
	signal(SIGUSR2, sighandl);

	/* connect to database */
	if (!arch_db_init(argv[2], argv[3], argv[4], argv[5], argv[6]))
		return EXIT_FAILURE;
	
	notify_init();
	
	if(notify_add_watch(argv[1]) == -1) {
		fprintf(stderr, "Invalid path: %s\n", argv[1]);
		return EXIT_FAILURE;
	}

    indexer_register(argv[1], NULL);
    
	arch_loop();
	
	return EXIT_SUCCESS;
}
