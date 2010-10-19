
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include "unit.h"
#include "../src/path.h"

void test_normalize() {

    char *ptr;

    ptr = path_normalize("usr/", "include/", 0);
    assert(ptr == NULL);

    ptr = path_normalize("/usr/src/", "linux", 0);
    assert_string(ptr, "/usr/src/linux");
    free(ptr);

    ptr = path_normalize("/segment1///segment2//", "segment3", 1);
    assert_string(ptr, "/segment1/segment2/segment3/");
    free(ptr);

    ptr = path_normalize("/stuff/with/ahell/lot/of/slashes/at/the///", "end", 1);
    assert_string(ptr, "/stuff/with/ahell/lot/of/slashes/at/the/end/");
    free(ptr);

    ptr = path_normalize("~", NULL, 0);
    printf("HOME EXPAND: %s\n", ptr);
    free(ptr);

    ptr = path_normalize("~/sub", "file", 0);
    printf("HOME EXPAND2: %s\n", ptr);
    free(ptr);

    ptr = path_normalize("/mnt/cdrom", "keff", 0);
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

void test_isabspath() {

    assert(is_abspath("file") == 0);
    assert(is_abspath("./file") == 0);
    assert(is_abspath(".file") == 0);
    assert(is_abspath("..file") == 0);
    assert(is_abspath("/../relpath") == 0);
    assert(is_abspath("/path/to/file") == 1);
    assert(is_abspath("/ab/xy/.file") == 1);
    assert(is_abspath("/ab/xy/..file") == 1);
    assert(is_abspath("/ab/.xy/file") == 1);
    assert(is_abspath("/ab/..xy/file") ==1);
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

    test_isabspath();
    test_normalize();
    test_pathlen();
    test_basename();
    test_dirname();

    return 0;
}
