CC = gcc
CTAGS = ctags

INCLUDES=-I/usr/local/include -I/usr/include -I${CSDK}
LIBS=-L/usr/local/lib -L/usr/lib -ltcmalloc -lcurl -lmagic
CFLAGS = -g -fPIC -c $(INCLUDES)

CSDK=./
SRCS = $(wildcard *.c)
OBJS = $(addsuffix .o, $(basename $(SRCS)))
HEADS = $(addsuffix .h, $(basename $(SRCS)))

all: $(OBJS)
	ar -r libufilecsdk.a $^

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $<

clean :
	find . -name '*.o' | xargs rm -f
	rm -rf libufilecsdk.a
