# see LICENSE

include config.mk

SRC = main.cc \
	  src/Loader.cc \
	  src/Logger.cc

OBJ = $(SRC:.cc=.o)

all: ykz

.cc.o:
	$(CXX) $(CXXFLAGS) -c $< -o $(basename $<).o

ykz: config.mk config.hpp $(OBJ)
	$(CXX) -o $@ $(CXXFLAGS) $(SRC:.cc=.o) $(LDFLAGS)

main.o: config.mk
Loader.o: config.mk Loader.hpp
Logger.o: config.mk Logger.hpp

config.hpp:
	cp config.def.hpp $@

clean:
	rm -f ykz main.o $(SRC:.cc=.o)
