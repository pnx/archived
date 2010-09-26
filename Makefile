#
# Archived Makefile
#

CC       = gcc
CFLAGS   = -O2 -Werror
LDFLAGS  = 

ifdef DEBUG
	CFLAGS += -g -D__DEBUG__
endif

ifndef V
	QUIET_CC = @echo '   ' CC $@;
	QUIET_LD = @echo '   ' LD $@;
endif

FINDOBJ = find . -name "*.o" -type f -printf "%P\n"

BUILD    := ./build
PROGRAM  := $(BUILD)/arch

ifeq ($(output), mysql)
	CFLAGS  += `mysql_config --cflags`
	LDFLAGS += -L/usr/lib/mysql -lmysqlclient
else
	output = stdout
endif

obj =

obj += src/ini/iniparser.o
obj += src/ini/dictionary.o

obj += src/common/rbtree.o
obj += src/common/path.o
obj += src/common/strbuf.o
obj += src/common/xalloc.o
obj += src/common/die.o

obj += src/output/$(output).o

obj += src/notify/inotify.o
obj += src/notify/event.o
obj += src/notify/tree.o
obj += src/notify/queue.o

obj += src/indexer.o
obj += src/arch.o

all : $(PROGRAM)

$(PROGRAM) : $(obj)
	@mkdir -p $(BUILD)
	$(QUIET_LD)$(CC) $(LDFLAGS) $^ -o $@

clean :
	@for obj in `$(FINDOBJ)`; do \
		echo $(RM) $$obj;$(RM) $$obj; \
	done
	
cleaner : clean
	$(RM) -r $(BUILD)

%.o : %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -c $< -o $@
