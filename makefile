CC=g++

Huffman: huffmain.o huffman.o
	$(CC) -o Huffman huffmain.o huffman.o

huffmain.o: huffmain.cpp huffman.h
	$(CC) -c -o huffmain.o huffmain.cpp

huffman.o: huffman.cpp huffman.h
	$(CC) -c -o huffman.o huffman.cpp
