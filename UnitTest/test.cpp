#include "pch.h"
#include "../Huffman/HuffmanEncoder.h"
#include "../Huffman/Sha256.h"
#include "../Huffman/BitCollector.h"

#include <filesystem>

TEST(GeneralTest, HuffmanTableBuilderTest)
{
	HuffmanEncoder::FrequencyContainer freq;
	freq['a'] = 19;
	freq['b'] = 21;
	freq['c'] = 2;
	freq['d'] = 3;
	freq['e'] = 6;
	freq['f'] = 7;
	freq['g'] = 10;
	freq['h'] = 32;

	auto huffmanTable = HuffmanEncoder::GenerateTreeFromFrequency(freq);

	EXPECT_EQ(huffmanTable.size(), 8);

	EXPECT_EQ(std::get<1>(huffmanTable['a']), 0b00);
	EXPECT_EQ(std::get<1>(huffmanTable['b']), 0b10);
	EXPECT_EQ(std::get<1>(huffmanTable['c']), 0b00001);
	EXPECT_EQ(std::get<1>(huffmanTable['d']), 0b10001);
	EXPECT_EQ(std::get<1>(huffmanTable['e']), 0b1001);
	EXPECT_EQ(std::get<1>(huffmanTable['f']), 0b0101);
	EXPECT_EQ(std::get<1>(huffmanTable['g']), 0b1101);
	EXPECT_EQ(std::get<1>(huffmanTable['h']), 0b11);

	EXPECT_EQ(std::get<0>(huffmanTable['a']), 2);
	EXPECT_EQ(std::get<0>(huffmanTable['b']), 2);
	EXPECT_EQ(std::get<0>(huffmanTable['c']), 5);
	EXPECT_EQ(std::get<0>(huffmanTable['d']), 5);
	EXPECT_EQ(std::get<0>(huffmanTable['e']), 4);
	EXPECT_EQ(std::get<0>(huffmanTable['f']), 4);
	EXPECT_EQ(std::get<0>(huffmanTable['g']), 4);
	EXPECT_EQ(std::get<0>(huffmanTable['h']), 2);

	HuffmanEncoder::Encode("test.txt","test.txt.huff");
	HuffmanEncoder::Decode("test.txt.huff", "test.txt.decode");
}

TEST(GeneralTest, HuffmanTableSerializeTest)
{
	HuffmanEncoder::FrequencyContainer freq;
	freq['a'] = 19;
	freq['b'] = 21;
	freq['c'] = 2;
	freq['d'] = 3;
	freq['e'] = 6;
	freq['f'] = 7;
	freq['g'] = 10;
	freq['h'] = 32;
	auto huffmanTable = HuffmanEncoder::GenerateTreeFromFrequency(freq);
	
	auto tableBuffer = HuffmanEncoder::SerializeHuffmanTable(huffmanTable);

	const std::string testFile{ "huffman-table.test" };
	if (std::filesystem::exists(testFile))
	{
		throw std::runtime_error(testFile + " already exists");
	}
	{
		std::ofstream ofs(testFile, std::ios::out | std::ios::binary);
		ofs.write(reinterpret_cast<const char*>(tableBuffer.data()), tableBuffer.size());
	}
	auto readBuffer = std::vector<uint8_t>();
	{
		std::ifstream ifs(testFile, std::ios::in | std::ios::binary);
		readBuffer.assign(std::istreambuf_iterator<char>(ifs),
						  std::istreambuf_iterator<char>());
	}
	std::filesystem::remove(testFile);
	auto unSerializedTable = HuffmanEncoder::UnSerializeHuffmanTable(readBuffer);

	EXPECT_EQ(unSerializedTable.size(), 8);

	EXPECT_EQ(std::get<1>(huffmanTable['a']), 0b00);
	EXPECT_EQ(std::get<1>(huffmanTable['b']), 0b10);
	EXPECT_EQ(std::get<1>(huffmanTable['c']), 0b00001);
	EXPECT_EQ(std::get<1>(huffmanTable['d']), 0b10001);
	EXPECT_EQ(std::get<1>(huffmanTable['e']), 0b1001);
	EXPECT_EQ(std::get<1>(huffmanTable['f']), 0b0101);
	EXPECT_EQ(std::get<1>(huffmanTable['g']), 0b1101);
	EXPECT_EQ(std::get<1>(huffmanTable['h']), 0b11);

	EXPECT_EQ(std::get<0>(unSerializedTable['a']), 2);
	EXPECT_EQ(std::get<0>(unSerializedTable['b']), 2);
	EXPECT_EQ(std::get<0>(unSerializedTable['c']), 5);
	EXPECT_EQ(std::get<0>(unSerializedTable['d']), 5);
	EXPECT_EQ(std::get<0>(unSerializedTable['e']), 4);
	EXPECT_EQ(std::get<0>(unSerializedTable['f']), 4);
	EXPECT_EQ(std::get<0>(unSerializedTable['g']), 4);
	EXPECT_EQ(std::get<0>(unSerializedTable['h']), 2);
}

TEST(GeneralTest, BitCollectorTest)
{
	std::vector<uint8_t> buffer;
	BitCollector collector(buffer);

	unsigned int tmp{ 10 };
	collector.Push(tmp, 0, 4);
	tmp = 0x00ff00ff;
	collector.Push(tmp, 8, 32);
	buffer.push_back(collector.Unpacked());
	
	EXPECT_EQ(collector.Unpacked(), 0);
	EXPECT_EQ(collector.RedundancyBit(), 4);
	
	EXPECT_EQ(buffer.size(), 4);
	EXPECT_EQ(buffer[0], 10);
	EXPECT_EQ(buffer[1], 240);
	EXPECT_EQ(buffer[2], 15);
	EXPECT_EQ(buffer[3], 0);
}