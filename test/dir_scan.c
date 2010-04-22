#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <time.h>
#include <string.h>


int main(int argc, char *argv[])
{
	time_t timer1, timer2;
	int count = 0;
	int output = 0;
	double avg;
	char *sep;
	char *base;
	char *path[] = {argv[1], NULL};
	FTS *tree;
	FTSENT *ent;
	

	if(atoi(argv[2]) == 1) {
		output = 1;
		printf("%s\n", argv[2]);
	}
	
	timer1 = time (NULL);
	
	tree = fts_open(path, FTS_PHYSICAL | FTS_NOSTAT, NULL);
	
	while ( ent = fts_read(tree) )
	{
		if (ent->fts_info != FTS_DP) 
		{
			if(output){
				
				base = malloc( strlen(ent->fts_path) - strlen(ent->fts_name) );
				sep = strrchr(ent->fts_path,'/');
				memcpy (base, ent->fts_path, (sep - ent->fts_path) );
				base[(sep - ent->fts_path)] = '\0';
				
				printf("Got: %s %s %i\n", base, ent->fts_name, ent->fts_level);
				
				free(base);
		
			}
			
			count++;
		}
	}
	
	fts_close(tree);
	
	timer2 = time (NULL);
	printf("Found: %i\n", count);
	printf("Start: %ld\n", timer1);
	printf("End: %ld\n", timer2);
	printf("Diff: %ld\n", timer2-timer1);
	if((timer2-timer1) > 0) {
		avg = count / (timer2-timer1);
		printf("Node/sec: %f\n", avg);
	}else{
		printf("Node/sec: inst.\n");
	}

	return 0;
}
