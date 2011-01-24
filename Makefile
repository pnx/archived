
#
# Archived Makefile
#

CC       = gcc
CFLAGS	 = -O2 -Werror -Ilib
LD		 = $(CC)
PROGRAM  := archived

-include Makefile.local.mk
include Makefile.include

ifdef DEBUG
	CFLAGS += -g -D__DEBUG__
endif

obj := $(obj-ini) $(obj-log) $(obj-notify) $(obj-path) \
	   $(obj-strbuf) $($obj-xalloc) $(obj-compat)

ifeq ($(database), mongo)
	LDFLAGS += -lmongoc -lbson
	obj += $(obj-mongo)
else
	CFLAGS += $(shell mysql_config --cflags)
	LDFLAGS += $(shell mysql_config --libs)
	obj += $(obj-mysql)
endif

.SUFFIXES: .c .o
.PHONY : clean distclean

all : $(PROGRAM)

$(PROGRAM) : src/archived.o $(obj)
	$(QUIET_LD)$(LD) $(sort $(^)) -o $@ $(LDFLAGS)

clean :
	@for obj in $(shell find . -name "*.o" -type f -printf "%P\n"); do \
		echo $(RM) $$obj;$(RM) $$obj; \
	done
	@make -C test clean
	@make -C docs clean

distclean : clean
	$(RM) $(PROGRAM)
	$(RM) Makefile.local.mk

%.o : %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -c $< -o $@

Makefile.local.mk :
	@echo " Can't find 'Makefile.local.mk'; copying default configuration"
	@cp Makefile.local.mk-dist Makefile.local.mk
