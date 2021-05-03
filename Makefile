# see LICENSE

include config.mk

SRC = main.cc \
	  src/Loader.cc

OBJ = $(SRC:.cc=.o)

all: ykz

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $(basename $<).o

ykz: config.mk config.h $(OBJ)
	$(CXX) -o $@ $(CXXFLAGS) $(SRC:.cc=.o) $(LDFLAGS)

main.o: config.mk
Loader.o: config.mk Loader.hpp

config.h:
	cp config.def.h $@

clean:
	rm -f ykz main.o $(SRC:.cc=.o)
