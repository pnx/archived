#
# Archived Makefile
#

CC       = gcc
CFLAGS   = -O2 -Werror -Ilib $(shell getconf LFS_CFLAGS)
LD		 = $(CC)
LDFLAGS  = $(shell getconf LFS_LDFLAGS)

FINDOBJ = find . -name "*.o" -type f -printf "%P\n"

PROGRAM  := archived

-include Makefile.local.mk

ifdef DEBUG
	CFLAGS += -g -D__DEBUG__
endif

ifndef VERBOSE
	QUIET_CC = @echo '   ' CC $@;
	QUIET_LD = @echo '   ' LD $@;
endif
ifeq ($(VERBOSE), 2)
	CFLAGS  += -v
endif

obj =

obj += lib/ini/iniparser.o
obj += lib/ini/dictionary.o

ifeq ($(database), mongo)
	LDFLAGS += -lmongoc -lbson
	obj += src/database/mongo.o
else
	CFLAGS += $(shell mysql_config --cflags)
	LDFLAGS += $(shell mysql_config --libs)
	obj += src/database/mysql.o
endif

obj += src/rbtree.o
obj += src/path.o
obj += src/strbuf.o
obj += src/xalloc.o
obj += src/die.o
obj += src/file.o

obj += src/inotify.o
obj += src/event.o
obj += src/fscrawl.o
obj += src/queue.o

obj += src/archived.o

.PHONY : all clean cleaner
all : $(PROGRAM)

$(PROGRAM) : $(obj)
	$(QUIET_LD)$(LD) $^ -o $@ $(LDFLAGS)

clean :
	@for obj in `$(FINDOBJ)`; do \
		echo $(RM) $$obj;$(RM) $$obj; \
	done

cleaner : clean
	$(RM) $(PROGRAM)

%.o : %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -c $< -o $@

Makefile.local.mk :
	@echo " Can't find 'Makefile.local.mk'; copying default configuration"
	@cp Makefile.local.mk-dist Makefile.local.mk
