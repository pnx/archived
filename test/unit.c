
#include "unit.h"

int utest_string(const char *a, const char *b) {
    
    while(*a == *b) {
        
        if (*a == 0)
            return 1;
   
        a++;
        b++;
    }
    
    return 0;
}
