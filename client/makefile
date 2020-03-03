.SUFFIXES:.c .o

CC=gcc

SRCS=ftclient.c\
	../common/ftcommon.c

CFLAGS=-Wall -g

OBJS=$(SRCS:.c=.o)
EXEC=ftclient

start: $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)
	@echo "--------ok--------"
.c.o:
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm $(EXEC) $(OBJS)