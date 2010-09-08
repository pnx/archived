#
# Archive Makefile
#

# uncomment and edit this to match your system
#DB_HOST := localhost
#DB_USER := dbusr
#DB_PASS := dbpass
#DB_NAME := dbname
#DB_TBL	 := dbtable

CC       := gcc
CFLAGS   := -Werror `mysql_config --cflags`
LDFLAGS  := -L/usr/lib/mysql -lmysqlclient

ifdef DEBUG
CFLAGS	 += -g -D__DEBUG__
endif

SOURCES  := \
	src/arch.c \
    src/arch/mysql.c \
	src/indexer.c \
	src/common/path.c \
	src/common/rbtree.c \
	src/notify/inotify.c \
	src/notify/event.c \
	src/notify/tree.c

BUILD    := ./build

all : 
	@mkdir -p $(BUILD)
ifdef DB_HOST 
	make wrapper
else
	@echo "Tip: edit this Makefile DB_* macros to enable generation of a wrapper shellscript"
endif
	$(CC) $(CFLAGS) $(LDFLAGS) $(SOURCES) -o $(BUILD)/arch
	
wrapper :
	@echo "#!/bin/sh" > $(BUILD)/wrapper
	@echo "if [ \$$# -lt 1 ]; then" >> $(BUILD)/wrapper 
	@echo "		echo \"usage: \$$0 <rootdir>\"" >> $(BUILD)/wrapper 
	@echo "		exit" >> $(BUILD)/wrapper 
	@echo "fi" >> $(BUILD)/wrapper 
	@echo "./arch \"\$$1\" \"$(DB_HOST)\" \"$(DB_USER)\" \"$(DB_PASS)\" \"$(DB_NAME)\" \"$(DB_TBL)\"" >> $(BUILD)/wrapper
	@chmod 755 $(BUILD)/wrapper
	@echo "generated $(BUILD)/wrapper"

clean :
	rm -fr $(BUILD)
