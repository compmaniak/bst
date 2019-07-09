CXX=g++
CXXFLAGS=-Wall -Wextra -Werror

all:
	$(CXX) -std=c++11 $(CXXFLAGS) -o bst bst.cpp

