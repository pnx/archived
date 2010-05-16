
#include <malloc.h>
#include <stdio.h>
#include "unit.h"
#include "../src/common/path.h"

void test_fmt_path() {

    char *ptr;

    ptr = fmt_path("usr/", "include/", 0);
    assert(ptr == NULL);

    ptr = fmt_path("/usr/src/", "linux", 0);
    assert_string(ptr, "/usr/src/linux");
    free(ptr);
/*
    ptr = fmt_path("/segment1/segment2/", "segment3/", 1);
    assert_string(ptr, "/segment1/segment2/segment3/");
    free(ptr);
    */

/*
    ptr = fmt_path("/stuff/with/ahell/lot/of/slashes/at/the/", "end/////////", 1);
    assert_string(ptr, "/stuff/with/ahell/lot/of/slashes/at/the/end/////////");
    free(ptr);
    */

    ptr = fmt_path("/mnt/cdrom", "keff", 0);
    assert_string(ptr, "/mnt/cdrom/keff");
    free(ptr);
}

void test_pathlen() {

    assert(pathlen("/usr/src/linux-2.6.30-r5/drivers") == 32);
    assert(pathlen("/usr///src/") == 9);
    assert(pathlen("/usr//include/sys//") == 17);
    assert(pathlen("///var/lib/misc") == 13);
    assert(pathlen("dir") == 3);
}

void test_basename() {

    int i;

    char data[11][2][64] = {
        { "", "." },
        { "/", "/" },
        { "///", "/" },
        { ".", "."},
        { "..", ".." },
        { "../../rel1", "rel1" },
        { "./rel2", "rel2" },
        { "justsomestring", "justsomestring" },
        { "/usr/src/", "src" },
        { "/usr/src///", "src" },
        { "/usr/src/linux-2.6.30-r5/drivers", "drivers" }
    };

    for(i=0; i < 11; i++)
        assert_string(basename(data[i][0]), data[i][1]);
}

void test_dirname() {

    int i;

    char data[11][2][64] = {
        { "", "." },
        { "/", "/" },
        { "///", "/" },
        { ".", "."},
        { "..", "." },
        { "../../rel", "../.." },
        { "./rel", "." },
        { "justsomestring", "." },
        { "/usr/src/", "/usr" },
        { "/usr/src///", "/usr" },
        { "/usr/src/linux-2.6.30-r5/drivers", "/usr/src/linux-2.6.30-r5" }
    };

    for(i=0; i < 11; i++)
        assert_string(dirname(data[i][0]), data[i][1]);
}

int main(int argc, char *argv[]) {

    test_fmt_path();
    test_pathlen();
    test_basename();
    test_dirname();

    return 0;
}
