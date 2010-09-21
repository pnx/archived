
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/output/output.h"
#include "../src/notify/event.h"

int main(int argc, char *argv[]) {

    int r;
    notify_event ev;

    ev.path = "/home/kalle/bilder";
    ev.filename = "kalle.jpg";
    ev.type = NOTIFY_CREATE;

	output_init(NULL);

    r = output_process(&ev);

    ev.type = NOTIFY_DELETE;

    r = output_process(&ev);

    if (r != 0) 
        printf("Error: %s\n", output_error(r));
        
    output_exit();

	return 0;
}
