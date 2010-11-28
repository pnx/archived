
#include "unit.h"
#include <stdlib.h>
#include <time.h>

static inline char ranchr() {

    char ch;

    do {
        ch = rand() % 0x7A;
    } while(ch < 0x20);

    return ch;
}

void __assert_str(const char *file, int line, const char *func, const char *a, const char *b) {
    
    if (a == NULL || b == NULL)
        __uexit(file, line, func, "a or b is null\n", NULL);

    if (strcmp(a, b) != 0)
        __uexit(file, line, func, "\"%s\" != \"%s\"\n", a, b);
    
}

void utest_init_RNG() {

    static unsigned char init = 0;
    
    if (init)
        return;
        
    srand(time(NULL));
    init = 1;
}

/*
 * Helper function for generating random strings
 */
char* utest_ran_string(size_t size) {

    int i, r;
    char *str;

    if (size < 1)
        return NULL;

    str = malloc(size+1);

    if (str == NULL)
        return NULL;

    utest_init_RNG();
    
    for(i=0; i < size; i++)
        str[i] = ranchr();
    str[i] = 0;

    return str;
}
