
#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include "../src/common/strbuf.h"

void print_strbuf(strbuf_t *s) {

    assert(s->len == strlen(s->buf));

    printf("block: %i, len: %i |%s|\n", s->alloc_size, s->len, s->buf);
}

int main() {

    strbuf_t b = STRBUF_INIT;
	char *str;

    strbuf_append(&b, "    ", 4);
    strbuf_append(&b, "abcdef", 6);
	
	print_strbuf(&b);
	
    strbuf_append(&b, "012345678901234567890123456789", 30);
    strbuf_append(&b, "      ", 6);

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

    strbuf_reduce(&b, 95);

    print_strbuf(&b);
	
    str = strbuf_release(&b);
	
	printf("released |%s|\n", str);
	
	free(str);

    return 0;
}
