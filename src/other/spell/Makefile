CC=gcc
CFLAGS = -g 

OBJS = newMain.o \
		good.o \
		tgood.o \
		lookup.o \
		makedent.o \
		hash.o 

spell: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f spell $(OBJS)
