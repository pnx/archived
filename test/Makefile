# Test makefile
DEFS= -D__DEBUG__
CC=gcc
CFLAGS=-g

all :
	make rbtree
	make inotify
	make mysql

raw_inotify :
	$(CC) -linotifytools raw_inotify.c -o raw_inotify

strbuf :
	$(CC) $(CFLAGS) ../src/common/strbuf.c t_strbuf.c -o test_strbuf

config :
	$(CC) ../src/common/config.c t_config.c -o test_config
	
path :
	$(CC) $(DEFS) $(CFLAGS) unit.c ../src/common/path.c t_path.c -o test_path

rbtree : 
	$(CC) $(DEFS) $(CFLAGS) unit.c ../src/common/rbtree.c t_rbtree.c -o test_rbtree

inotify :
	$(CC) -lpthread -D INOTIFY_DEBUG -D RB_DEBUG $(CFLAGS) \
	    ../src/common/rbtree.c \
	    ../src/common/path.c \
	    ../src/fs/scan.c \
	    ../src/fs/notify_event.c \
	    ../src/fs/inotify.c \
	t_inotify.c -o test_inotify

tree :
	$(CC) $(CFLAGS) $(DEFS) ../src/common/path.c ../src/fs/tree.c t_tree.c -o test_tree
	
indexer :
	$(CC) $(CFLAGS) -D__DEBUG__ ../src/common/path.c ../src/fs/tree.c ../src/indexer.c t_indexer.c -o test_indexer

mysql :
	$(CC) -D DB_DEBUG $(CFLAGS) ../src/mysql_db.c -L/usr/lib64/mysql \ 
		-lmysqlclient -lz -lcrypt -lnsl -lm -L/usr/lib64 -lssl -lcrypto t_mysql.c -o test_mysql

queue :
	$(CC) $(CFLAGS) ../src/fs/queue.c t_queue.c -o test_queue
