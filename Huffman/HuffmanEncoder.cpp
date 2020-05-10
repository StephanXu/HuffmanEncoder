#include "pch.h"
#include "HuffmanEncoder.h"
#include "BitCollector.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <stack>
#include <set>
#include <unordered_map>

auto HuffmanEncoder::Encode(const std::string& sourceFilename, const std::string& destination) -> void
{
	std::ifstream fs{sourceFilename, std::ios::in | std::ios::binary};
	std::ofstream output{destination, std::ios::out | std::ios::binary};
	if (!fs.is_open())
	{
		throw std::runtime_error("Encode: Can't open source file");
	}
	if (!output.is_open())
	{
		throw std::runtime_error("Encode: Can't open output file");
	}
	Encode(fs, output);
}

auto HuffmanEncoder::Encode(std::istream& source, std::ostream& destination) -> void
{
	auto [hash, frequency] = GetFrequencyAndHash(source);
	auto huffmanTable      = GenerateTreeFromFrequency(frequency);

	// Reset the status of file.
	source.clear();
	source.seekg(std::ios::beg);

	// Prepare meta data.
	auto serializedTable     = SerializeHuffmanTable(huffmanTable);
	auto metaData            = SerializedHuffmanTableMetaData{};
	metaData.m_TableLength   = serializedTable.size();
	metaData.m_RedundancyBit = 0;
	std::copy(hash.begin(), hash.end(), metaData.m_FileHash);

	destination.write(reinterpret_cast<const char*>(&metaData), sizeof(metaData));

	// Write Huffman table
	destination.write(reinterpret_cast<const char*>(serializedTable.data()), serializedTable.size());

	// Encode
	auto writeBuffer  = std::vector<uint8_t>();
	auto bitCollector = BitCollector(writeBuffer);
	auto endPos       = std::istreambuf_iterator<char>();
	for (auto readPos{std::istreambuf_iterator<char>(source)}; readPos != endPos; ++readPos)
	{
		auto it = huffmanTable.find(*readPos);
		if (it == huffmanTable.end())
		{
			throw std::runtime_error("Unknown character");
		}
		const auto& [bitLength, encode] = it->second;

		bitCollector.Push(encode, 0, bitLength);

		if (writeBuffer.size() >= 512)
		{
			destination.write(reinterpret_cast<const char*>(writeBuffer.data()), writeBuffer.size());
			writeBuffer.clear();
		}
	}
	writeBuffer.push_back(bitCollector.Unpacked());
	destination.write(reinterpret_cast<const char*>(writeBuffer.data()), writeBuffer.size());
	writeBuffer.clear();

	// Update meta data.
	metaData.m_RedundancyBit = static_cast<uint8_t>(bitCollector.RedundancyBit());
	destination.seekp(std::ios::beg);
	destination.write(reinterpret_cast<const char*>(&metaData), sizeof(metaData));
}

auto HuffmanEncoder::Decode(const std::string& sourceFilename,
                            const std::string& destination) -> std::vector<unsigned char>
{
	std::ifstream fs{sourceFilename, std::ios::in | std::ios::binary};
	std::ofstream output{destination, std::ios::out | std::ios::binary};
	if (!fs.is_open())
	{
		throw std::runtime_error("Decode: Can't open source file");
	}
	if (!output.is_open())
	{
		throw std::runtime_error("Decode: Can't open output file");
	}
	return Decode(fs, output);
}

auto HuffmanEncoder::Decode(std::istream& source, std::ostream& destination) -> std::vector<unsigned char>
{
	source.seekg(-1, std::ios::end);
	const auto lastBytePos = source.tellg();
	source.seekg(std::ios::beg);

	// Get meta data
	SerializedHuffmanTableMetaData metaData{};
	source.read(reinterpret_cast<char*>(&metaData), sizeof(metaData));

	// UnSerialize huffman table
	std::vector<uint8_t> huffmanTableBuffer(metaData.m_TableLength);
	source.read(reinterpret_cast<char*>(huffmanTableBuffer.data()), metaData.m_TableLength);
	auto huffmanTable = UnSerializeDecodeHuffmanTable(huffmanTableBuffer);

	// Decode file
	const size_t unitSize        = 8;
	auto writeBuffer             = std::vector<uint8_t>();
	auto endPos                  = std::istreambuf_iterator<char>();
	const uint8_t mask[unitSize] = {0x1, 0x3, 0x7, 0xF, 0x1f, 0x3f, 0x7f, 0xff};
	uint8_t bitPos               = 0;
	unsigned int encode          = 0;
	size_t bitLength             = 1;
	for (auto readPos{std::istreambuf_iterator<char>(source)}; readPos != endPos; ++readPos)
	{
		const bool isLastByte = source.tellg() == lastBytePos;
		const uint8_t current = *readPos;
		for (size_t i{}; i < (isLastByte ? metaData.m_RedundancyBit : unitSize); ++i)
		{
			if (bitPos > sizeof(uint32_t) * CHAR_BIT)
			{
				throw std::runtime_error("Cannot support encode longer than 32 bit");
			}
			encode |= ((current >> i) & mask[0]) << bitPos;
			auto lengthIt = huffmanTable.find(bitLength);
			if (lengthIt == huffmanTable.end())
			{
				++bitLength;
				++bitPos;
				continue;
			}
			auto it = lengthIt->second.find(encode);
			if (it == lengthIt->second.end())
			{
				++bitLength;
				++bitPos;
				continue;
			}
			writeBuffer.push_back(it->second);
			bitLength = 1;
			bitPos    = 0;
			encode    = 0;
		}

		if (writeBuffer.size() >= 512)
		{
			destination.write(reinterpret_cast<const char*>(writeBuffer.data()), writeBuffer.size());
			writeBuffer.clear();
		}
	}
	destination.write(reinterpret_cast<const char*>(writeBuffer.data()), writeBuffer.size());

	auto digest = std::vector<unsigned char>{
		metaData.m_FileHash,
		metaData.m_FileHash + picosha2::k_digest_size
	};
	return digest;
}

auto HuffmanEncoder::Verify(const std::string& sourceFilename, const std::vector<unsigned char>& digest) -> bool
{
	std::ifstream fs{sourceFilename, std::ios::in | std::ios::binary};
	if (!fs.is_open())
	{
		throw std::runtime_error("Verify: Can't open file");
	}
	return Verify(fs, digest);
}

auto HuffmanEncoder::Verify(std::istream& source, const std::vector<unsigned char>& digest) -> bool
{
	if (digest.size() != picosha2::k_digest_size)
	{
		return false;
	}
	std::vector<unsigned char> actualDigest(picosha2::k_digest_size);
	picosha2::hash256(std::istreambuf_iterator<char>(source),
	                  std::istreambuf_iterator<char>(),
	                  actualDigest.begin(),
	                  actualDigest.end());
	for (size_t i{}; i < picosha2::k_digest_size; ++i)
	{
		if (digest[i] != actualDigest[i])
		{
			return false;
		}
	}
	return true;
}

auto HuffmanEncoder::GetMetaData(std::istream& source) -> std::tuple<SerializedHuffmanTableMetaData, HuffmanTableMap>
{
	auto result                    = std::make_tuple(SerializedHuffmanTableMetaData{}, HuffmanTableMap{});
	auto& [metaData, huffmanTable] = result;

	// Get meta data
	source.read(reinterpret_cast<char*>(&metaData), sizeof(SerializedHuffmanTableMetaData));

	// UnSerialize huffman table
	std::vector<uint8_t> huffmanTableBuffer(metaData.m_TableLength);
	source.read(reinterpret_cast<char*>(huffmanTableBuffer.data()), metaData.m_TableLength);
	huffmanTable = UnSerializeHuffmanTable(huffmanTableBuffer);

	return result;
}

auto HuffmanEncoder::GetMetaData(
	const std::string& filename) -> std::tuple<SerializedHuffmanTableMetaData, HuffmanTableMap>
{
	std::ifstream fs(filename, std::ios::in | std::ios::binary);
	if (!fs.is_open())
	{
		throw std::runtime_error("GetMetaData: Can't open file");
	}
	return GetMetaData(fs);
}

auto HuffmanEncoder::GetFrequencyAndHash(
	std::istream& fileStream) -> std::tuple<std::vector<unsigned char>, FrequencyContainer>
{
	const size_t bufferSize{PICOSHA2_BUFFER_SIZE_FOR_INPUT_ITERATOR};
	auto result = std::make_tuple(std::vector<unsigned char>(picosha2::k_digest_size), FrequencyContainer());
	auto& [digest, frequency] = result;

	std::vector<unsigned char> buffer(bufferSize);
	picosha2::hash256_one_by_one hasher;
	const auto end = std::istreambuf_iterator<char>();
	for (auto readPos = std::istreambuf_iterator<char>(fileStream); readPos != end;)
	{
		size_t actualSize{bufferSize};
		for (size_t i{}; i != bufferSize; ++i, ++readPos)
		{
			if (end == readPos)
			{
				actualSize = i;
				break;
			}
			frequency[*readPos] += 1; ///< Add frequency
			buffer[i] = *readPos;
		}
		hasher.process(buffer.begin(), buffer.begin() + actualSize);
	}
	hasher.finish();
	hasher.get_hash_bytes(digest.begin(), digest.end());

	return result;
}

auto HuffmanEncoder::GenerateTreeFromFrequency(const FrequencyContainer& frequency) -> HuffmanTableMap
{
	const auto comparison = [](const std::shared_ptr<HuffmanTreeNode>& lhs,
	                           const std::shared_ptr<HuffmanTreeNode>& rhs)-> bool
	{
		return lhs->m_Frequency < rhs->m_Frequency;
	};
	std::multiset<std::shared_ptr<HuffmanTreeNode>, decltype(comparison)> frequencySet{comparison};
	for (const auto& item : frequency)
	{
		frequencySet.insert(std::make_shared<HuffmanTreeNode>(
			HuffmanTreeNode{{}, {}, item.second, item.first}));
	}
	while (1 < frequencySet.size())
	{
		auto left = *frequencySet.begin();
		frequencySet.erase(frequencySet.begin());
		auto right = *frequencySet.begin();
		frequencySet.erase(frequencySet.begin());

		frequencySet.insert(std::make_shared<HuffmanTreeNode>(
			HuffmanTreeNode{left, right, left->m_Frequency + right->m_Frequency, 0}));
	}
	auto rootNode = *frequencySet.begin();

	HuffmanTableMap huffmanTable;
	std::stack<std::tuple<std::shared_ptr<HuffmanTreeNode>, std::vector<unsigned char>>> stack;
	std::vector<unsigned char> path;
	auto node = rootNode;
	while (node || !stack.empty())
	{
		while (node)
		{
			stack.push(std::make_tuple(node, path));
			if (!node->m_LeftChild)
			{
				unsigned int encode{};
				for (size_t i{0}; i != path.size(); ++i)
				{
					//encode |= path[i] << (path.size() - 1 - i);
					encode |= path[i] << i; ///< Reversed huffman encode.
				}
				huffmanTable[node->m_Character] = std::make_tuple(path.size(), encode);
			}
			else
			{
				path.push_back(0);
			}
			node = node->m_LeftChild;
		}

		if (!stack.empty())
		{
			node = std::get<0>(stack.top());
			path = std::get<1>(stack.top());
			if (node->m_RightChild)
			{
				path.push_back(1);
			}
			node = node->m_RightChild;
			stack.pop();
		}
	}

	return huffmanTable;
#ifdef _DEBUG
	for (const auto& item : huffmanTable)
	{
		std::cout << item.first << "\t";
		auto [bitLength, encode] = item.second;
		for (int i{static_cast<int>(bitLength) - 1}; i >= 0; --i)
		{
			std::cout << ((encode & (0 | (1 << i))) >> i);
		}
		std::cout << "\t" << encode;
		std::cout << std::endl;
	}
#endif
}

auto HuffmanEncoder::SerializeHuffmanTable(const HuffmanTableMap& huffmanTable) -> std::vector<uint8_t>
{
	std::vector<uint8_t> buffer;
	buffer.reserve(sizeof(SerializedHuffmanTableItem) * huffmanTable.size());
	SerializedHuffmanTableItem huffmanTableItem{};

	for (const auto& item : huffmanTable)
	{
		const auto& [character, pair]   = item;
		const auto& [bitLength, encode] = pair;
		huffmanTableItem.m_Character    = character;
		if (bitLength > (std::numeric_limits<uint8_t>::max)())
		{
			throw(std::overflow_error("Huffman bitLength overflow."));
		}
		huffmanTableItem.m_BitLength = static_cast<uint8_t>(bitLength);

		if (encode > (std::numeric_limits<uint16_t>::max)())
		{
			throw(std::overflow_error("Huffman encode overflow."));
		}
		huffmanTableItem.m_Encode = static_cast<uint16_t>(encode);

		std::copy(reinterpret_cast<uint8_t*>(&huffmanTableItem),
		          reinterpret_cast<uint8_t*>(&huffmanTableItem) + sizeof(SerializedHuffmanTableItem),
		          std::back_inserter(buffer));
	}

	return buffer;
}

auto HuffmanEncoder::UnSerializeHuffmanTable(const uint8_t* buffer, const size_t length) -> HuffmanTableMap
{
	if (0 != length % sizeof(SerializedHuffmanTableItem))
	{
		throw std::invalid_argument("Invalid huffman table buffer");
	}

	HuffmanTableMap huffmanTable;

	for (const auto* readPos = reinterpret_cast<const SerializedHuffmanTableItem*>(buffer);
	     readPos != reinterpret_cast<const SerializedHuffmanTableItem*>(buffer + length);
	     ++readPos)
	{
		huffmanTable.insert(std::make_pair(readPos->m_Character,
		                                   std::make_tuple(readPos->m_BitLength, readPos->m_Encode)));
	}

	return huffmanTable;
}

auto HuffmanEncoder::UnSerializeHuffmanTable(const std::vector<uint8_t>& buffer) -> HuffmanTableMap
{
	return UnSerializeHuffmanTable(buffer.data(), buffer.size());
}

auto HuffmanEncoder::UnSerializeDecodeHuffmanTable(const uint8_t* buffer,
                                                   const size_t length) -> HuffmanTableDecodeMap
{
	if (0 != length % sizeof(SerializedHuffmanTableItem))
	{
		throw std::invalid_argument("Invalid huffman table buffer");
	}

	HuffmanTableDecodeMap huffmanTable;

	for (const auto* readPos = reinterpret_cast<const SerializedHuffmanTableItem*>(buffer);
	     readPos != reinterpret_cast<const SerializedHuffmanTableItem*>(buffer + length);
	     ++readPos)
	{
		huffmanTable[readPos->m_BitLength].insert(std::make_pair(readPos->m_Encode, readPos->m_Character));
	}

	return huffmanTable;
}

auto HuffmanEncoder::UnSerializeDecodeHuffmanTable(const std::vector<uint8_t>& buffer) -> HuffmanTableDecodeMap
{
	return UnSerializeDecodeHuffmanTable(buffer.data(), buffer.size());
}
