#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <string>

/*
 * Takes in an inFile as ascii test and outputs an encoded file
*  outFile using huffman code.
 */
void huffmanEncode(const char* inFile, const char* outFile);

/*
 * Takes an inFile and decodes it's huffman encoding and outputs
 * the decoded text to outfile.
 */
void huffmanDecode(const char* inFile, const char* outFile);

#endif
