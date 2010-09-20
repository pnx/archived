# Test makefile
CC=gcc
CFLAGS=-g -D__DEBUG__ 
LDFLAGS=-L/usr/lib64/mysql -lmysqlclient

all : raw_inotify strbuf path rbtree inotify tree indexer mysql queue

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
	$(CC) $(CFLAGS) unit.c ../src/common/rbtree.c t_rbtree.c -o test_rbtree

inotify :
	$(CC) -lpthread -D INOTIFY_DEBUG -D RB_DEBUG $(CFLAGS) \
	    ../src/common/rbtree.c \
	    ../src/common/xalloc.c \
		../src/common/die.c \
		../src/common/strbuf.c \
	    ../src/common/path.c \
	    ../src/notify/event.c \
	    ../src/notify/tree.c \
	    ../src/notify/inotify.c \
	t_inotify.c -o test_inotify

tree :
	$(CC) $(CFLAGS) \
		../src/common/path.c \
		../src/notify/tree.c \
		../src/common/xalloc.c \
		../src/common/die.c \
		../src/common/strbuf.c \
	t_tree.c -o test_tree
	
indexer :
	$(CC) $(CFLAGS) $(LDFLAGS) \
		../src/common/rbtree.c \
		../src/common/path.c \
		../src/common/xalloc.c \
		../src/common/die.c \
		../src/common/strbuf.c \
		../src/notify/tree.c \
		../src/notify/inotify.c \
		../src/notify/event.c \
		../src/arch/mysql.c \
		../src/indexer.c \
	t_indexer.c -o test_indexer

mysql :
	$(CC) -D DB_DEBUG $(CFLAGS) $(LDFLAGS) \
		../src/output/mysql.c \
		../src/common/die.c \
		../src/common/xalloc.c \
		../src/ini/iniparser.c \
		../src/ini/dictionary.c \
	t_mysql.c -o test_mysql

queue :
	$(CC) $(CFLAGS) ../src/notify/queue.c t_queue.c -o test_queue

clean :
	rm -f test_*
