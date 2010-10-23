
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "util.h"

int file_exists(const char *path) {

    struct stat st;

    if (stat(path, &st) < 0)
        return 0;

    return S_ISREG(st.st_mode);
}
