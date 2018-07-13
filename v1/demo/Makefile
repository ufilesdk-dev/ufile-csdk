CC = gcc
MAKE = make
CTAGS = ctags

INCLUDES=-I/usr/local/include -I/usr/include -I../ucloud/ -I/use/local/include/json-c
LIBS = -L../ucloud/ -L/usr/local/lib -L/usr/lib -lcurl -lmagic -ljson-c -lufilecsdk
CFLAGS = -g -fPIC -c $(INCLUDES)
OFLAGS = -g -Wall $(LIBS)

SRCS = $(wildcard ./*.c)
OBJS = $(addsuffix .o, $(basename $(SRCS)))
HEADS = $(addsuffix .h, $(basename $(SRCS)))

APP=demo
$(APP): $(OBJS)
	$(MAKE) -C ../ucloud/
	$(CC) $^ -o $@ $(OFLAGS)

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $<

clean :
	rm -f $(APP) *.o
	$(MAKE) -C ../ucloud/ clean
