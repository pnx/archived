
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../src/strbuf.h"

typedef unsigned int uint;

void print_strbuf(strbuf_t *s) {

    assert(s->len == strlen(s->buf));

    printf("block: %u, len: %u |%s|\n", (uint)s->alloc_size, (uint)s->len, s->buf);
}

void test_setlen() {

    strbuf_t b = STRBUF_INIT;

    strbuf_setlen(&b, 25);
    assert(b.len == 0);
    
    strbuf_expand(&b, 25);
    strbuf_setlen(&b, b.alloc_size);
    assert(b.len == b.alloc_size-1);

    strbuf_setlen(&b, 0);
    assert(b.len == 0);

    strbuf_append_str(&b, "testing...");
    print_strbuf(&b);

    strbuf_setlen(&b, 7);
    assert(b.len == 7);

    print_strbuf(&b);

    strbuf_free(&b);
}

void test_appendf() {

    strbuf_t b = STRBUF_INIT;

    strbuf_appendf(&b, "%s %i", "string", 32);

    print_strbuf(&b);

    strbuf_free(&b);
}

void test_release() {

    strbuf_t b = STRBUF_INIT;
    char *str;

    strbuf_append_str(&b, "release");

    print_strbuf(&b);

    str = strbuf_release(&b);
	
	printf("released |%s|\n", str);
	
	free(str);
}

void test_release_empty() {

    strbuf_t b = STRBUF_INIT;

    char *ptr = strbuf_release(&b);

    assert(*ptr == '\0');

    free(ptr);
}

void test_squeeze() {

    strbuf_t b = STRBUF_INIT;

    strbuf_squeeze(&b, 'X');

    strbuf_append_str(&b, "aaabXXXcdefXXXXghijklXXmmmnopXXXXXqrXstuXXvwxyXXzXX");
    strbuf_squeeze(&b, 'X');
    print_strbuf(&b);
    strbuf_free(&b);

    strbuf_append_str(&b, "XXXX");
    strbuf_squeeze(&b, 'X');
    print_strbuf(&b);
    strbuf_free(&b);

    strbuf_append_str(&b, "A");
    strbuf_squeeze(&b, 'X');
    print_strbuf(&b);
    strbuf_free(&b);
}

void test_chop() {

    strbuf_t b = STRBUF_INIT;

    strbuf_rchop(&b, 'X');
    print_strbuf(&b);

    strbuf_append_str(&b, "X");
    
    strbuf_rchop(&b, 'X');
    print_strbuf(&b);

    strbuf_append_str(&b, "123456789X123456789");

    strbuf_rchop(&b, 'X');
    print_strbuf(&b);

    strbuf_rchop(&b, 'X');
    print_strbuf(&b);
    
    strbuf_free(&b);
}

void test_term() {

    strbuf_t b = STRBUF_INIT;
    strbuf_t c = STRBUF_INIT;
    strbuf_append_str(&b, "test");
    strbuf_append_str(&c, "test.");
    strbuf_term(&b, '.');
    strbuf_term(&c, '.');
    assert(strcmp(b.buf, c.buf) == 0);
    strbuf_free(&b);
    strbuf_free(&c);
}

void test_trim() {

    strbuf_t b = STRBUF_INIT;
	
    print_strbuf(&b);

    strbuf_append_repeat(&b, ' ', 4);
    strbuf_append(&b, "abcdef", 6);
	
	print_strbuf(&b);
	
    strbuf_append(&b, "012345678901234567890123456789", 30);
    strbuf_append_ch(&b, 'a'); 
    strbuf_append_repeat(&b, ' ', 6);

    print_strbuf(&b);

    strbuf_rtrim(&b);

    print_strbuf(&b);
	
	strbuf_ltrim(&b);
	
	print_strbuf(&b);

    strbuf_free(&b);
}

void test_rev() {

    strbuf_t b = STRBUF_INIT;

    strbuf_append_str(&b, "reversed");

    print_strbuf(&b);

	strbuf_rev(&b);
	
	print_strbuf(&b);

    strbuf_free(&b);
}

void test_reduce() {

    strbuf_t b = STRBUF_INIT;

    strbuf_append_str(&b, "reduce...");

    print_strbuf(&b);

    strbuf_reduce(&b, 3);

    print_strbuf(&b);

    strbuf_reduce(&b, 256);

    print_strbuf(&b);

    strbuf_free(&b);
}

void test_explode() {

    strbuf_t **list, **ptr;
    strbuf_t b = STRBUF_INIT;

    strbuf_append_str(&b, "item1,item2,item3,item4");

    list = ptr = strbuf_explode(&b, ',');
    while(*ptr)
        print_strbuf(*ptr++);
    strbuf_free_list(list);

    strbuf_free(&b);

    list = strbuf_explode(&b, ',');

    assert(*list == NULL);

    strbuf_free_list(list);
}

void test_free_empty() {

    strbuf_t b = STRBUF_INIT;

    strbuf_free(&b);
}

int main() {

    test_appendf();
    test_release_empty();
    test_release();
    test_setlen();
    test_reduce();
    test_rev();
    test_squeeze();
    test_trim();
    test_term();
    test_chop();
    test_explode();
    test_free_empty();
    
    return 0;
}
