# Test makefile
CC=gcc
CFLAGS=-g -D__DEBUG__ 
LDFLAGS=-L/usr/lib64/mysql -lmysqlclient

all : raw_inotify strbuf path rbtree inotify fscrawl queue

raw_inotify :
	$(CC) -linotifytools t_raw_inotify.c -o test_raw_inotify

strbuf :
	$(CC) $(CFLAGS) \
		../src/common/strbuf.c \
		../src/common/xalloc.c \
		../src/common/die.c \
	t_strbuf.c -o test_strbuf
	
path :
	$(CC) $(CFLAGS) \
		unit.c \
		../src/common/xalloc.c \
		../src/common/die.c \
		../src/common/strbuf.c \
		../src/common/path.c \
	t_path.c -o test_path

rbtree : 
	$(CC) $(CFLAGS) \
		unit.c \
		../src/common/die.c \
		../src/common/xalloc.c \
		../src/common/rbtree.c \
	t_rbtree.c -o test_rbtree

inotify :
	$(CC) -lpthread -D INOTIFY_DEBUG -D RB_DEBUG $(CFLAGS) \
	    ../src/common/rbtree.c \
	    ../src/common/xalloc.c \
		../src/common/die.c \
		../src/common/strbuf.c \
	    ../src/common/path.c \
	    ../src/notify/event.c \
		../src/notify/fscrawl.c \
	    ../src/notify/queue.c \
	    ../src/notify/inotify.c \
	t_inotify.c -o test_inotify

fscrawl :
	$(CC) $(CFLAGS) \
		../src/common/path.c \
		../src/notify/fscrawl.c \
		../src/common/xalloc.c \
		../src/common/die.c \
		../src/common/strbuf.c \
	t_fscrawl.c -o test_fscrawl

queue :
	$(CC) $(CFLAGS) ../src/notify/queue.c t_queue.c -o test_queue

clean :
	rm -f test_*
