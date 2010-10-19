#
# Archived Makefile
#

CC       = gcc
CFLAGS   = -O2 -Werror
LD		 = $(CC)
LDFLAGS  = 

FINDOBJ = find . -name "*.o" -type f -printf "%P\n"

BUILD    := build
PROGRAM  := $(BUILD)/archived

include Makefile.local.mk

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

ifeq ($(DEBUG), 2)
	obj += src/client/stdout.o
else
	CFLAGS  += `mysql_config --cflags`
	LDFLAGS += -L/usr/lib/mysql -lmysqlclient
	obj += src/ini/iniparser.o
	obj += src/ini/dictionary.o
	obj += src/client/mysql.o
endif

obj += src/rbtree.o
obj += src/path.o
obj += src/strbuf.o
obj += src/xalloc.o
obj += src/die.o

obj += src/inotify.o
obj += src/event.o
obj += src/fscrawl.o
obj += src/queue.o

.PHONY : all clean cleaner
all : $(PROGRAM)

$(PROGRAM) : $(obj)
	@mkdir -p $(BUILD)
	$(QUIET_LD)$(LD) $(LDFLAGS) $^ -o $@

clean :
	@for obj in `$(FINDOBJ)`; do \
		echo $(RM) $$obj;$(RM) $$obj; \
	done

cleaner : clean
	$(RM) -r $(BUILD)

%.o : %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -c $< -o $@

Makefile.local.mk :
	@echo " Can't find 'Makefile.local.mk'; copying default configuration"
	@cp Makefile.local.mk-dist Makefile.local.mk
