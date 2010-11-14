
ASCIIDOC = asciidoc

SOURCE_DOCS = $(subst index.txt,,$(wildcard *.txt))

HTML_DOCS = index.html $(patsubst %.txt,%.html,$(SOURCE_DOCS))

ifndef VERBOSE
	QUIET_ASCIIDOC = @echo '   ' GEN $@;
	QUIET_SCRIPT   = @echo '   ' GEN $@;
endif

.PHONY : clean
all : html
html : $(HTML_DOCS)

%.html : %.txt
	$(QUIET_ASCIIDOC)$(ASCIIDOC) -b xhtml11 -o $@ $<

index.txt : $(SOURCE_DOCS)
	$(QUIET_SCRIPT)$(shell ./genindex.sh index.txt)

clean :
	$(RM) index.txt *.html
