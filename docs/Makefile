
ASCIIDOC = asciidoc

HTML_DOCS = api-notify.html strbuf.html xalloc.html

ifndef VERBOSE
	QUIET_ASCIIDOC = @echo '   ' GEN $@;
endif

.PHONY : clean
all : $(HTML_DOCS)
html : $(HTML_DOCS)

%.html : %.txt
	$(QUIET_ASCIIDOC)$(ASCIIDOC) -b xhtml11 -o $@ $<

clean :
	$(RM) $(HTML_DOCS)
