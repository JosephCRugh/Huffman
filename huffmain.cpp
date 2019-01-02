/*
 * A simple huffman encoder/decoder for files.
 *  Compiled and ran on arch linux.
 *
 * Use:
 *   ./Huffman encode [inFile] [outFile]
 *   ./Huffman decode [inFile] [outFile]
 *
 * This project was created by Joseph C. Rugh.
 */
#include "huffman.h"
#include <iostream>
#include <algorithm>
#include <fstream>

void printOptionalCalls();

int main(int argc, char** argv)
{

  if (argc < 4)
  {
    printOptionalCalls();
    return 1;
  }

  std::string action = argv[1];
  std::transform(action.begin(), action.end(), action.begin(), ::tolower);

  if (action == "encode")
  {
    huffmanEncode(argv[2], argv[3]);
  }
  else if (action == "decode")
  {
    huffmanDecode(argv[2], argv[3]);
  }
  else
  {
    printOptionalCalls();
    return 1;
  }

  return 0;
}

void printOptionalCalls()
{
  std::cout << "Optional calls:\n";
  std::cout << "Huffman encode [inFile] [outFile]\n";
  std::cout << "Huffman decode [inFile] [outFile]\n";
}
