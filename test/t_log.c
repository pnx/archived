
#include <errno.h>
#include <stddef.h>
#include "../src/log.h"

int main() {

    init_log(LOG_INFO | LOG_WARN, NULL);

    logmsg(LOG_INFO, "this is stderr");

    logmsg(LOG_CRIT, "Should not show");
    logerrno(LOG_CRIT, NULL, ENOENT);

    init_log(LOG_INFO | LOG_WARN | LOG_CRIT, "./logs/");

    logmsg(LOG_INFO, "some info");
    logmsg(LOG_WARN, "invalid type '%i'", 3);

    logerrno(LOG_CRIT, "malloc", ENOMEM);
    logerrno(LOG_CRIT, NULL, ENOENT);
    
    logmsg(LOG_DEBUG, "Should not show");

    logmsg(LOG_INFO | LOG_CRIT, "Should not work, can only log to one priority");

    return 0;
}
