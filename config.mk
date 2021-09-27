PREFIX   = /usr/local

CXX       = clang++
CXXFLAGS += -I. -std=c++17 -pedantic -Wall -Wextra -g3
#we could do with 14- just replace string_view with char[] or somth
#CXXFLAGS = -std=c++17 -pedantic -Wall -Wextra -Os -Iinclude/
LDFLAGS  += -lpthread -ldl

