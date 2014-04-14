
#
# Archived Makefile
#
all::

CC     = gcc
CFLAGS = -O2 -Wall -Ilib
LD     = $(CC)
PROGRAM  := archived

-include Makefile.local.mk
include Makefile.include

ifdef DEBUG
	CFLAGS += -g -D__DEBUG__
endif

obj := $(obj-ini) $(obj-log) $(obj-notify) $(obj-path) \
	$(obj-strbuf) $($obj-xalloc) $(obj-compat)

CFLAGS  += $(shell mysql_config --cflags)
LDFLAGS += $(shell mysql_config --libs)
obj     += $(obj-backend-mysql)

VERSION :
	@$(SHELL) ./VERSION-GEN > VERSION
-include VERSION

.SUFFIXES: .c .o
.PHONY : docs clean distclean

all:: $(PROGRAM)

$(PROGRAM) : src/archived.o $(obj)
	$(QUIET_LD)$(LD) $(sort $(^)) -o $@ $(LDFLAGS)

docs :
	$(MAKE) -C docs all

clean :
	@for obj in $(shell find . -name "*.o" -type f -printf "%P\n"); do \
		echo $(RM) $$obj;$(RM) $$obj; \
	done
	@make -C test clean
	@make -C docs clean
	$(RM) VERSION

distclean : clean
	$(RM) $(PROGRAM)
	$(RM) Makefile.local.mk

src/archived.o : CFLAGS += -DARCHIVED_VERSION='"$(ARCHIVED_VERSION)"'

%.o : %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -c $< -o $@

Makefile.local.mk :
	@echo " Can't find 'Makefile.local.mk'; copying default configuration"
	@cp Makefile.local.mk-dist Makefile.local.mk
