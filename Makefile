# see LICENSE

include config.mk

SRC = queue misc server

all: ykz

ykz: config.mk config.h main.o $(SRC:=.o)
	$(CC) -o $@ $(CFLAGS) main.o $(SRC:=.o) $(LDFLAGS)

main.o: config.mk main.c 
queue.o: config.mk queue.c queue.h
server.o: config.mk server.c server.h
misc.o: config.mk misc.c misc.h 

config.h:
	cp config.def.h $@

clean:
	rm -f ykz main.o $(SRC:=.o)
