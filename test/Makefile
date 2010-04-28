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
	
path :
	$(CC) -D__DEBUG__ $(CFLAGS) ../src/common/path.c t_path.c -o test_path

path2 :
	$(CC) -D__DEBUG__ $(CFLAGS) ../src/common/path.c t_path2.c -o test_path2

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
