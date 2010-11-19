# Test makefile
CC=gcc
CFLAGS=-g -D__DEBUG__
LDFLAGS=-L/usr/lib64/mysql -lmysqlclient

all : raw_inotify strbuf path rbtree inotify fscrawl queue

raw_inotify :
	$(CC) -linotifytools t_raw_inotify.c -o test_raw_inotify

strbuf :
	$(CC) $(CFLAGS) \
		../src/strbuf.c \
		../src/xalloc.c \
		../src/die.c \
	t_strbuf.c -o test_strbuf
	
path :
	$(CC) $(CFLAGS) \
		unit.c \
		../src/xalloc.c \
		../src/die.c \
		../src/strbuf.c \
		../src/path.c \
	t_path.c -o test_path

rbtree : 
	$(CC) $(CFLAGS) \
		unit.c \
		../src/die.c \
		../src/xalloc.c \
		../src/rbtree.c \
	t_rbtree.c -o test_rbtree

inotify :
	$(CC) -lpthread -D INOTIFY_DEBUG -D RB_DEBUG $(CFLAGS) \
	    ../src/rbtree.c \
	    ../src/xalloc.c \
		../src/die.c \
		../src/strbuf.c \
	    ../src/path.c \
	    ../src/event.c \
		../src/fscrawl.c \
	    ../src/queue.c \
	    ../src/inotify.c \
	t_notify.c -o test_inotify

fscrawl :
	$(CC) $(CFLAGS) \
		../src/path.c \
		../src/fscrawl.c \
		../src/xalloc.c \
		../src/die.c \
		../src/strbuf.c \
	t_fscrawl.c -o test_fscrawl

queue :
	$(CC) $(CFLAGS) ../src/queue.c t_queue.c -o test_queue

log :
	$(CC) $(CFLAGS) ../src/die.c ../src/strbuf.c ../src/xalloc.c ../src/log.c t_log.c -o test_log

clean :
	rm -f test_*
