# see LICENSE

include config.mk

SRC = main.cc src/Loader.cc src/Logger.cc src/Config.cc
OBJ = $(SRC:.cc=.o)

all: options ykz

options:
	@echo ykz build options;
	@echo "CXX      = ${CXX}"
	@echo "CXXFLAGS = ${CXXFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"

re: clean all

.cc.o:
	${CXX} ${CXXFLAGS} -c $< -o ${basename $<}.o

ykz: config.mk config.h ${OBJ}
	${CXX} -o $@ ${CXXFLAGS} ${OBJ} ${LDFLAGS}

main.o: config.mk
Loader.o: config.mk Loader.hpp
Logger.o: config.mk Logger.hpp
Config.cc: config.mk Config.hpp

config.h:
	cp config.def.h $@

clean:
	rm -f ykz main.o ${SRC:.cc=.o}

.PHONY: all options clean re
