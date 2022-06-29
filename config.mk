PREFIX   = /usr/local

CXX       = clang++
CXXFLAGS += -I. -std=c++17 -pedantic -Wall -Wextra -g3
LDFLAGS  += -lpthread -ldl
