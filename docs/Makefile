
ASCIIDOC = asciidoc

DOCS = $(filter-out index.txt, $(wildcard *.txt))
TECH_DOCS = $(filter-out technical/index.txt, $(wildcard technical/*.txt))

ifndef VERBOSE
	QUIET_ASCIIDOC = @echo '   ' GEN $@;
	QUIET_SCRIPT   = @echo '   ' GEN $@;
endif

.PHONY : clean

all : html

html : index.html

technical : technical/index.html

%.html : %.txt
	$(QUIET_ASCIIDOC)$(ASCIIDOC) -b xhtml11 -o $@ $<

index.txt : $(patsubst %.txt,%.html,$(DOCS))
	$(QUIET_SCRIPT)$(shell ./genindex.sh index.txt)

technical/index.txt : $(patsubst %.txt,%.html,$(TECH_DOCS))
	$(QUIET_SCRIPT)cd technical && ./genindex.sh index.txt

clean :
	$(RM) index.txt *.html technical/index.txt technical/*.html
