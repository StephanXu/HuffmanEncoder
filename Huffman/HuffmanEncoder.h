#ifndef HUFFMAN_ENCODER_H
#define HUFFMAN_ENCODER_H
#pragma once

#include "Sha256.h"

#include <memory>
#include <vector>
#include <unordered_map>

#ifndef FRIEND_TEST
#define FRIEND_TEST(x,y)
#endif

/// <summary>
/// Huffman encode functions
/// </summary>
class HuffmanEncoder
{
	FRIEND_TEST(GeneralTest, HuffmanTableBuilderTest);
	FRIEND_TEST(GeneralTest, HuffmanTableSerializeTest);

public:
	HuffmanEncoder() = delete;

	using FrequencyContainer = std::unordered_map<char, unsigned int>;
	using HuffmanTableMap = std::unordered_map<char, std::tuple<size_t, unsigned int>>;
	using HuffmanTableDecodeMap = std::unordered_map<size_t, std::unordered_map<unsigned int, char>>;

	/// <summary>
	/// The metadata of compressed file.
	/// </summary>
	struct SerializedHuffmanTableMetaData
	{
		uint64_t m_TableLength;
		uint8_t m_RedundancyBit;
		uint8_t m_FileHash[picosha2::k_digest_size];
	};

	/// <summary>
	/// Encoding a file.
	/// </summary>
	/// <param name="sourceFilename">Filename of source file</param>
	/// <param name="destination">Destination of encoded file</param>
	/// <returns>void</returns>
	static auto Encode(const std::string& sourceFilename, const std::string& destination) -> void;

	/// <summary>
	/// Encoding a stream.
	/// </summary>
	/// <param name="source">Stream source</param>
	/// <param name="destination">Output destination</param>
	/// <returns>void</returns>
	static auto Encode(std::istream& source, std::ostream& destination) -> void;

	/// <summary>
	/// Decode a file.
	/// </summary>
	/// <param name="sourceFilename">File name of source file</param>
	/// <param name="destination">Destination of decoded file</param>
	/// <returns>void</returns>
	static auto Decode(const std::string& sourceFilename, const std::string& destination) -> std::vector<unsigned char>;

	/// <summary>
	/// Decode a stream
	/// </summary>
	/// <param name="source">Stream source</param>
	/// <param name="destination">Output destination</param>
	/// <returns></returns>
	static auto Decode(std::istream& source, std::ostream& destination) -> std::vector<unsigned char>;

	/// <summary>
	/// Verify a file with SHA-256 digest.
	/// </summary>
	/// <param name="sourceFilename">The file to verify</param>
	/// <param name="digest">SHA-256 digest</param>
	/// <returns>True if the input file's digest and input digest are same</returns>
	static auto Verify(const std::string& sourceFilename, const std::vector<unsigned char>& digest) -> bool;

	/// <summary>
	/// Verify a stream content with SHA-256 digest.
	/// </summary>
	/// <param name="source">Stream source</param>
	/// <param name="digest">SHA-256 digest</param>
	/// <returns>True if the input file's digest and input digest are same</returns>
	static auto Verify(std::istream& source, const std::vector<unsigned char>& digest) -> bool;

	/// <summary>
	/// Return the metadata from stream.
	/// </summary>
	/// <param name="source">Stream source</param>
	/// <returns>{metadata, huffmanTable}</returns>
	static auto GetMetaData(std::istream& source)->std::tuple<SerializedHuffmanTableMetaData, HuffmanTableMap>;

	/// <summary>
	/// Return the metadata from file.
	/// </summary>
	/// <param name="filename">Filename to read</param>
	/// <returns>{metadata, huffmanTable}</returns>
	static auto GetMetaData(const std::string& filename)->std::tuple<SerializedHuffmanTableMetaData, HuffmanTableMap>;

private:

	/// <summary>
	/// Get frequency table and hash of a file.
	/// In order to avoid unnecessary access of file..
	/// </summary>
	/// <param name="fileStream">Source stream</param>
	/// <returns>{digest, frequencyTable}</returns>
	static auto GetFrequencyAndHash(std::istream& fileStream)
	-> std::tuple<std::vector<unsigned char>, FrequencyContainer>;

	/// <summary>
	/// The huffman tree node struct
	/// </summary>
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

	static auto SerializeHuffmanTable(const HuffmanTableMap& huffmanTable) -> std::vector<uint8_t>;

	static auto UnSerializeHuffmanTable(const uint8_t* buffer, const size_t length) -> HuffmanTableMap;

	static auto UnSerializeHuffmanTable(const std::vector<uint8_t>& buffer) -> HuffmanTableMap;

	static auto UnSerializeDecodeHuffmanTable(const uint8_t* buffer, const size_t length) -> HuffmanTableDecodeMap;

	static auto UnSerializeDecodeHuffmanTable(const std::vector<uint8_t>& buffer) -> HuffmanTableDecodeMap;
};


#endif // HUFFMAN_ENCODER_H
