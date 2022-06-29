LIBOG    = deps/libOG
CXX      = clang++
CXXFLAGS = -I. -I${LIBOG} -std=c++11 -pedantic -Wall -Wextra -g3 -gdwarf-4
LDFLAGS  = -lpthread -ldl -L${LIBOG} -lOG
