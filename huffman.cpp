#include "huffman.h"
#include <sstream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <limits>
#include <iostream>

/*
 * A huffman node used to encode the data.
 */
struct hnode
{
	std::size_t value;
	hnode* lbn,
		   * rbn;
	std::string encoding;
};

hnode* findSmallestNode(std::vector<hnode*>& topNodes);

void treeTraversal(hnode* topNode, std::string binary = "");

std::unordered_map<char, hnode*> calculateFrequences(std::string msg);

void outputToFile(std::string msg, const char* fileName, std::unordered_map<char, hnode*> occurences);

void byteConvert(std::string bitStr, bool lastString);

std::vector<std::string> splitString(std::string str, char deli);

std::vector<char> sizetToBuffer(size_t size);

size_t bufferToSizet(char* buffer);

// Global conversion data
int           bytePos  = 0;     // The bit position of the current byte being encoded.
unsigned char curByte  = 0x00;  // The current byte being encoded.
size_t        numChs   = 0;     // The number of characters within the file being read in.
std::vector<char> binaryData;   // The 0/1 representation of characters encoding.

void huffmanEncode(const char* inFile, const char* outFile)
{
  std::ifstream inStream(inFile);
  if (inStream.fail())
  {
    perror(inFile);
    return;
  }
  std::string msg((std::istreambuf_iterator<char>(inStream)),
                      (std::istreambuf_iterator<char>()));
	inStream.close();

	if (msg.empty()) return;

	std::unordered_map<char, hnode*> occurences = calculateFrequences(msg);
	std::vector<hnode*> topNodes;
	for (const auto& entry : occurences) topNodes.push_back(entry.second);

	// Generating a tree structure of nodes.
	bool nodesLeftToLink = true;
	hnode* topNode = nullptr;
	while (nodesLeftToLink)
	{
		hnode* fsm = findSmallestNode(topNodes);
		if (fsm != nullptr) topNodes.erase(std::remove(topNodes.begin(), topNodes.end(), fsm), topNodes.end()); else break;
		hnode* ssm = findSmallestNode(topNodes);
		if (ssm != nullptr) topNodes.erase(std::remove(topNodes.begin(), topNodes.end(), ssm), topNodes.end()); else break;

		// Linking a new node
		nodesLeftToLink = fsm != ssm;
		if (nodesLeftToLink)
			topNodes.push_back(topNode = new hnode { fsm->value + ssm->value, fsm, ssm, "" });
	}

	treeTraversal(topNode);

	for (int i = 0; i < msg.length(); i++)
	{
		byteConvert(occurences[msg[i]]->encoding, i + 1 == msg.length());
	}

	outputToFile(msg, outFile, occurences);

	bytePos = 0;
	curByte = 0x00;
	numChs = 0;
	binaryData.clear();
}

std::unordered_map<char, hnode*> calculateFrequences(std::string msg)
{
	std::unordered_map<char, hnode*> occurences;
	// We begin by finding frequencies.
	for (char& ch : msg)
		if (occurences.find(ch) == occurences.end())
			occurences.insert(std::pair<char, hnode*>(ch, new hnode { 1, nullptr, nullptr, "" }));
		else occurences[ch]->value++;
	return occurences;
}

hnode* findSmallestNode(std::vector<hnode*>& topNodes)
{
	hnode* sm = nullptr;
	for (const auto& n : topNodes)
		if (sm == nullptr) sm = n;
		else if (n->value < sm->value) sm = n;
	return sm;
}

void treeTraversal(hnode* topNode, std::string binary)
{
	// traverse left
	if (topNode->lbn != nullptr)
	{
		treeTraversal(topNode->lbn, binary + "0");
		treeTraversal(topNode->rbn, binary + "1");
		delete topNode;
		return;
	}

	// Reached the bottom of the tree
	topNode->encoding = binary;
}

void outputToFile(std::string msg, const char* fileName, std::unordered_map<char, hnode*> occurences)
{
	std::ofstream outStream(fileName, std::ios::binary | std::ios::out);
  if (outStream.fail())
  {
    perror(fileName);
    return;
  }

	// Writing a header to indicate it is a huffman encoded file.
	char headerSig[] = { (char)0x1F, (char)0xA0 };
	outStream.write(headerSig, 2);

	std::vector<char> tableSizeBuffer = sizetToBuffer(occurences.size());
	outStream.write(&tableSizeBuffer[0], 4);

  // converting the binary strings into binary
	size_t count = 0;
	size_t binaryEncodings[occurences.size()];
	for (const auto& entry : occurences)
	{
		std::string binaryStr = entry.second->encoding;
		size_t binary = 0x00;
		size_t lengthOfEncoding = 0;
		for (char ch : binaryStr)
		{
			binary |= ch == '1' ? 1 : 0;
			binary <<= 1;
			lengthOfEncoding++;
		}
		// Adding the length of the encoded binary to the end of the encoding.
		binary |= lengthOfEncoding << 24;
		binaryEncodings[count++] = binary;
		delete entry.second;
	}

	// Packing the data to be written into a buffer.
	char tableBuffer[occurences.size() * 5];
	count = 0;
	for (const auto& entry : occurences)
	{
		size_t offset = count * 5;
		tableBuffer[offset + 0] = entry.first;
		std::vector<char> binaryToPack = sizetToBuffer(binaryEncodings[count]);
		tableBuffer[offset + 1] = binaryToPack[0];
		tableBuffer[offset + 2] = binaryToPack[1];
		tableBuffer[offset + 3] = binaryToPack[2];
		tableBuffer[offset + 4] = binaryToPack[3];
		count++;
	}
	outStream.write(tableBuffer, occurences.size() * 5);

	std::vector<char> binSizeBuffer = sizetToBuffer(binaryData.size());
	std::vector<char> chSizeBuffer = sizetToBuffer(numChs);

	outStream.write(&chSizeBuffer[0], 4);
	outStream.write(&binSizeBuffer[0], 4);
	outStream.write(&binaryData[0], binaryData.size());

	outStream.close();
}

std::vector<char> sizetToBuffer(size_t size)
{
	std::vector<char> buffer(4);
	buffer[0] = (size >> 24) & 0xFF;
	buffer[1] = (size >> 16) & 0xFF;
	buffer[2] = (size >> 8) & 0xFF;
	buffer[3] = size & 0xFF;
	return buffer;
}

void huffmanDecode(const char* inFile, const char* outFile)
{
	// The file with the encoded data.
  std::ifstream inStream(inFile, std::ios::binary | std::ios::in);
  if (inStream.fail())
  {
    perror(inFile);
    return;
  }

	char headerSig[2];
	inStream.read(headerSig, 2);

	if (headerSig[0] != (char)0x1F || headerSig[1] != (char)0xA0)
	{
		std::cerr << "The file is not a huffman compressed file. (Missing signature)\n";
		return;
	}

	char tableBufferSize[4];
	inStream.read(tableBufferSize, 4);
	size_t tableSize = bufferToSizet(tableBufferSize);

	char tableBuffer[tableSize * 5];
	inStream.read(tableBuffer, tableSize * 5);

	// Reading in the encoding table.
	char shorestCode = 127;
	std::unordered_map<std::string, char> chEncodings;
	for (size_t i = 0; i < tableSize; i++)
	{
		size_t offset = i * 5;
		char ch = tableBuffer[offset];
		size_t binary = bufferToSizet(&tableBuffer[offset + 1]);
		std::string strEncoding = "";
		// The length of the code's bits.
		char codeSize = (binary >> 24) & 0xFF;
		shorestCode = std::min(shorestCode, codeSize);

		for (int i = codeSize; i > 0; i--)
			strEncoding += (binary >> i) & 0x01 == 0x01 ? '1' : '0';

		// Used to lookup the encodings.
		chEncodings.insert(std::pair<std::string, char>(strEncoding, ch));
	}

	char chBufferSize[4];
	inStream.read(chBufferSize, 4);
	size_t numChars = bufferToSizet(chBufferSize);

	char binBufferSize[4];
	inStream.read(binBufferSize, 4);
	size_t binSize = bufferToSizet(binBufferSize);

	char binaryBytes[binSize];
	inStream.read(binaryBytes, binSize);

	// Loops through the binary and finds matching encoding
	// that represents the characters. The characters are constructed
	// into a string to be written out to a file.
	std::string fullEncoding = "", code = "";
	size_t charCount = 0;
	for (char c : binaryBytes)
	{
		for (int j = 7; j >= 0; j--)
	    {
	      code += ((c >> j) & 0x01) ? '1' : '0';
				if (code.length() >= shorestCode)
				{
	        if (chEncodings.find(code) != chEncodings.end())
					{
						if (charCount > numChars)
						{
							break;
						}
	          fullEncoding += chEncodings[code];
						code = "";
						charCount++;
	        }
	      }
	    }
	}

  inStream.close();

	// Writing the decoded string to a file.
	std::ofstream outStream(outFile);
	if (outStream.fail())
	{
		perror(outFile);
		return;
	}

	outStream << fullEncoding;

	outStream.close();

	return;
}

size_t bufferToSizet(char* buffer)
{
	size_t size = buffer[0] & 0xFF;
	for (int i = 1; i < 4; i++)
	{
		size <<= 8;
		size |= buffer[i] & 0xFF;
	}
	return size;
}

void byteConvert(std::string bitStr, bool lastString)
{
	numChs++;

#define BYTE_SHIFT(binary) { 															                            	\
	unsigned char byte = 0;																	                              \
	for (int i = 0; i < binary.length(); i++) if (binary[i] == '1') byte |= 1 << (7-i);		\
	if (bytePos > 0) byte >>= bytePos;														                        \
	curByte |= byte; }

#define PUSH_BACK_BYTE() { 				\
	bytePos = 0;						        \
	binaryData.push_back(curByte);	\
	curByte = 0x00; }

	while (!bitStr.empty())
	{
		// This means we need to shift and re-evaluate
		if (bytePos + bitStr.length() > 8)
		{
			int rmPos = 8 - bytePos;
			std::string binary = bitStr.substr(0, rmPos);
			bitStr = bitStr.substr(rmPos);

			BYTE_SHIFT(binary);

			PUSH_BACK_BYTE();
		}
		else
		{
			BYTE_SHIFT(bitStr);

			bytePos += bitStr.length();


			if (bytePos == 8)
			{
				PUSH_BACK_BYTE();
			}
			else if (lastString)
			{
				PUSH_BACK_BYTE();
			}
			break;
		}
	}

#undef BYTE_SHIFT
#undef PUSH_BACK_BYTE
}

std::vector<std::string> splitString(std::string str, char deli)
{
	std::vector<std::string> seglist;
	std::string segment;
	std::stringstream ss(str);
	while(std::getline(ss, segment, deli)) seglist.push_back(segment);
	return seglist;
}
