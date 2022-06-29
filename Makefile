# see LICENSE

include config.mk

SRC = main.cc \
	  Loader.cc \
	  Logger.cc \
	  TaskBucket.cc

OBJ = $(SRC:.cc=.o)

all: options ykz

.cc.o:
	${CXX} ${CXXFLAGS} -c $< -o ${basename $<}.o

main.o: config.mk
Loader.o: config.mk Loader.hpp
Logger.o: config.mk Logger.hpp
TaskBucket.o: config.mk TaskBucket.hpp

options:
	@echo ykz build options:
	@echo "CXX      = ${CXX}"
	@echo "CXXFLAGS = ${CXXFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"

re: clean all

ykz: config.mk ${OBJ}
	${CXX} -o $@ ${CXXFLAGS} ${OBJ} ${LDFLAGS}

clean:
	rm -f ykz ${SRC:.cc=.o}

.PHONY: all options clean re
