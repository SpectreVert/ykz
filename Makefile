# see LICENSE

include config.mk

SRC = queue utils

all: ykz

ykz: config.mk config.h main.o $(SRC:=.o)
	$(CC) -o $@ $(CFLAGS) main.o $(SRC:=.o) $(LDFLAGS)

main.o: config.mk main.c 
queue.o: config.mk queue.c queue.h
utils.o: config.mk utils.c utils.h 

config.h:
	cp config.def.h $@

clean:
	rm -f ykz main.o $(SRC:=.o)
