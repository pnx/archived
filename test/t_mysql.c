
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/output/output.h"
#include "../src/ini/iniparser.h"
#include "../src/notify/event.h"


int main(int argc, char *argv[]) {
	
	printf("Run!\n");

        printf("Here 1\n");
        dictionary *config = iniparser_load("config.ini");

        printf("Here 2\n");
        notify_event ev;

        printf("Here 3\n");
        ev.path = "/home/kalle/bilder";
        ev.filename = "kalle.jpg";
        ev.type = NOTIFY_CREATE;

        printf("Here 4\n");
	output_init(config);

        printf("Here 5\n");
        int r = output_process(&ev);

        printf("Here 6\n");
        if (r != 0) {
            printf("Error:\n");
            printf(output_error(r));
        }

        printf("Here 7\n");
        output_exit();

        printf("Here 8\n");
	
	return 0;
}