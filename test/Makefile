# Test makefile

CC		= gcc
CFLAGS  = -g -D__DEBUG__
LD      = $(CC)
LDFLAGS =

include ../Makefile.include

ROOT = ../
TESTS = $(patsubst t_%.c,%,$(wildcard t_*.c))

.PHONY : clean
all : $(TESTS)

strbuf : $(addprefix $(ROOT),$(obj-strbuf))
rbtree : unit.c $(addprefix $(ROOT),$(obj-rbtree))
queue : $(ROOT)src/queue.o $(addprefix $(ROOT),$(obj-xalloc))
path : unit.c $(ROOT)src/path.o $(addprefix $(ROOT),$(obj-strbuf))
fscrawl : $(ROOT)src/fscrawl.o $(addprefix $(ROOT),$(obj-fscrawl))
notify : $(addprefix $(ROOT),$(obj-notify))
log : $(addprefix $(ROOT),$(obj-log))
str-list : unit.c $(addprefix $(ROOT),$(obj-str-list))

clean :
	$(RM) $(patsubst t_%.c,test_%,$(wildcard t_*.c))

% : t_%.c
	$(QUIET_LD)$(LD) $(LDFLAGS) $(sort $(^)) -o test_$@

%.o : %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -c $< -o $@
