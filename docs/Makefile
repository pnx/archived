
ASCIIDOC = asciidoc

HTML_DOCS = index.html api-notify.html strbuf.html xalloc.html rbtree.html \
			log.html fscrawl.html path.html

ifndef VERBOSE
	QUIET_ASCIIDOC = @echo '   ' GEN $@;
	QUIET_SCRIPT   = @echo '   ' GEN $@;
endif

.PHONY : clean
all : $(HTML_DOCS)
html : $(HTML_DOCS)

%.html : %.txt
	$(QUIET_ASCIIDOC)$(ASCIIDOC) -b xhtml11 -o $@ $<

index.txt :
	$(QUIET_SCRIPT)$(shell ./genindex.sh index.txt)

clean :
	$(RM) index.txt $(HTML_DOCS)
