# see LICENSE

include config.mk

OBJ = main.o\
	  Host.o\
	  utils.o\

all: options ykz

ykz: config.mk ${OBJ}
	make -C ${LIBOG}
	${CXX} -o $@ ${CXXFLAGS} ${OBJ} ${LDFLAGS}

.cc.o:
	${CXX} ${CXXFLAGS} -c $< -o ${basename $<}.o

main.o: Protocol.hpp config.mk ykz.config.hpp
Host.o: Host.hpp Protocol.hpp config.mk ykz.config.hpp
utils.o: utils.hpp Protocol.hpp config.mk ykz.config.hpp

options:
	@echo ykz build options:
	@echo "CXX      = ${CXX}"
	@echo "CXXFLAGS = ${CXXFLAGS}"
	@echo "LDFLAGS  = ${LDFLAGS}"

re: clean all

clean:
	rm -f ykz ${OBJ}
	make clean -C ${LIBOG}

.PHONY: all options clean re
