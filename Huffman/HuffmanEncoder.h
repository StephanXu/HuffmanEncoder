#ifndef HUFFMAN_ENCODER_H
#define HUFFMAN_ENCODER_H
#pragma once

#include "Sha256.h"

#include <memory>
#include <vector>
#include <unordered_map>


#include <bitset>


#ifndef FRIEND_TEST
#define FRIEND_TEST(x,y)
#endif

class HuffmanEncoder
{
	FRIEND_TEST(GeneralTest, HuffmanTableBuilderTest);
	FRIEND_TEST(GeneralTest, HuffmanTableSerializeTest);

public:
	HuffmanEncoder() = delete;

	using FrequencyContainer = std::unordered_map<char, unsigned int>;
	using HuffmanTableMap = std::unordered_map<char, std::tuple<size_t, unsigned int>>;
	using HuffmanTableDecodeMap = std::unordered_map<size_t, std::unordered_map<unsigned int, char>>;

	static auto Encode(const std::string& sourceFilename, const std::string& destination) -> void;

	static auto Encode(std::istream& source, std::ostream& destination) -> void;

	static auto Decode(const std::string& sourceFilename, const std::string& destination) -> std::vector<unsigned char>;

	static auto Decode(std::istream& source, std::ostream& destination) -> std::vector<unsigned char>;

	static auto Verify(const std::string& sourceFilename, const std::vector<unsigned char>& digest) -> bool;

	static auto Verify(std::istream& source, const std::vector<unsigned char>& digest) -> bool;

private:
	static auto GetFrequencyAndHash(std::istream& fileStream)
	-> std::tuple<std::vector<unsigned char>, FrequencyContainer>;

	struct HuffmanTreeNode
	{
		std::shared_ptr<HuffmanTreeNode> m_LeftChild;
		std::shared_ptr<HuffmanTreeNode> m_RightChild;
		unsigned int m_Frequency;
		char m_Character;
	};

	static auto GenerateTreeFromFrequency(const FrequencyContainer& frequency) -> HuffmanTableMap;

	struct SerializedHuffmanTableItem
	{
		uint8_t m_Character;
		uint8_t m_BitLength;
		uint16_t m_Encode;
	};

	struct SerializedHuffmanTableMetaData
	{
		uint64_t m_TableLength;
		uint8_t m_RedundancyBit;
		uint8_t m_FileHash[picosha2::k_digest_size];
	};

	static auto SerializeHuffmanTable(const HuffmanTableMap& huffmanTable) -> std::vector<uint8_t>;

	static auto UnSerializeHuffmanTable(const uint8_t* buffer, const size_t length) -> HuffmanTableMap;

	static auto UnSerializeHuffmanTable(const std::vector<uint8_t>& buffer) -> HuffmanTableMap;

	static auto UnSerializeDecodeHuffmanTable(const uint8_t* buffer, const size_t length) -> HuffmanTableDecodeMap;

	static auto UnSerializeDecodeHuffmanTable(const std::vector<uint8_t>& buffer) -> HuffmanTableDecodeMap;
};


#endif // HUFFMAN_ENCODER_H
