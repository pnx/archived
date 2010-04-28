#
# Archive Makefile
#

DEFS	 := -D__DEBUG__
LIBS     := `mysql_config --libs`
CFLAGS   := -Werror -g $(LIBS) $(DEFS)
CC       := gcc

SOURCES  := \
	src/arch.c \
    src/arch/mysql.c \
    src/indexer.c \
	src/common/path.c \
	src/common/rbtree.c \
	src/fs/notify_inotify.c \
	src/fs/notify_event.c \
	src/fs/tree.c

BUILD    := ./build

all : 
	make me

me :
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(DEFS) $(SOURCES) -o $(BUILD)/arch

clean :
	rm -fr $(BUILD)
