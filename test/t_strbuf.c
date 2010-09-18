
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "../src/common/strbuf.h"

typedef unsigned int uint;

void print_strbuf(strbuf_t *s) {

    assert(s->len == strlen(s->buf));

    printf("block: %u, len: %u |%s|\n", (uint)s->alloc_size, (uint)s->len, s->buf);
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

    strbuf_append_str(&b, "aaabXXXcdefXXXXghijklXXmmmnopXXXXXqrstuXXvwxyXXzXX");
    strbuf_squeeze(&b, 'X');
    print_strbuf(&b);
    strbuf_free(&b);

    strbuf_append_str(&b, "XXXX");
    strbuf_squeeze(&b, 'X');
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

void test() {

    strbuf_t b = STRBUF_INIT;
	
    print_strbuf(&b);

    strbuf_append(&b, "    ", 4);
    strbuf_append(&b, "abcdef", 6);
	
	print_strbuf(&b);
	
    strbuf_append(&b, "012345678901234567890123456789", 30);
    strbuf_append_ch(&b, 'a'); 
    strbuf_append_str(&b, "      ");

    print_strbuf(&b);

    strbuf_rtrim(&b);

    print_strbuf(&b);
	
	strbuf_ltrim(&b);

    print_strbuf(&b);
	
	strbuf_trim(&b);
	
	print_strbuf(&b);

	strbuf_rev(&b);
	
	print_strbuf(&b);

    strbuf_reduce(&b, 6);

    print_strbuf(&b);

    /* testing release */
    char *str = strbuf_release(&b);
	
	printf("released |%s|\n", str);
	
	free(str);
}

int main() {

    test();
    
    test_release_empty();

    test_squeeze();

    test_term();
    
    return 0;
}
