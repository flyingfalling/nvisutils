

default: test.exe

test.exe: byte4color.hpp event.hpp node.hpp property.hpp visdefs.hpp
	g++ -std=c++11 -o test.exe test.cpp string_tokenizer.cpp -I. -I..
