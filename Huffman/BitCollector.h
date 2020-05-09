#pragma once
#ifndef BIT_COLLECTOR_H
#define BIT_COLLECTOR_H

#include <vector>

class BitCollector
{
	using UnitType = uint8_t;

	constexpr static auto UnitBit() -> size_t { return CHAR_BIT * sizeof(UnitType); }

public:
	explicit BitCollector(std::vector<uint8_t>& container)
		: m_Buffer(container)
	{
	}

	template <typename T>
	auto Push(const T value, const size_t beginPos, const size_t endPos) -> void
	{
		static_assert(std::is_pod_v<T>, "Type T should be POD type");
		const auto valueBitLength = sizeof(T) * CHAR_BIT;
		if (valueBitLength < beginPos || valueBitLength < endPos || beginPos > endPos)
		{
			throw std::out_of_range("BitCollector::Push out of range.");
		}

		for (auto i = beginPos / UnitBit();
			 i < endPos / UnitBit();
			 ++i)
		{
			PushImpl(reinterpret_cast<const UnitType*>(&value)[i],
					 i > beginPos / UnitBit() ? 0 : beginPos % UnitBit(),
					 UnitBit());
		}
		if (valueBitLength != endPos)
		{
			PushImpl(reinterpret_cast<const UnitType*>(&value)[endPos / UnitBit()], 0, endPos % UnitBit());
		}
	}

	auto Unpacked() const noexcept -> UnitType { return m_Unpacked; }

	auto RedundancyBit() const noexcept -> size_t { return m_RedundancyBit; }

private:
	UnitType m_Unpacked{};
	size_t m_RedundancyBit{};
	std::vector<UnitType>& m_Buffer;

	auto PushImpl(const UnitType value, const size_t beginPos, const size_t endPos) -> void
	{
		if (endPos == beginPos)
		{
			return;
		}
		const UnitType mask[8] = { 0x1, 0x3, 0x7, 0xF, 0x1f, 0x3f, 0x7f, 0xff };
		const UnitType actualValue = (value >> beginPos) & mask[endPos - beginPos - 1];
		m_Unpacked |= actualValue << m_RedundancyBit;
		m_RedundancyBit += endPos - beginPos;
		if (m_RedundancyBit > UnitBit())
		{
			m_Buffer.push_back(m_Unpacked);
			m_Unpacked = 0;
			m_Unpacked |= actualValue >> (UnitBit() - (m_RedundancyBit - (endPos - beginPos)));
			m_RedundancyBit -= UnitBit();
		}
	}
};

#endif // BIT_COLLECTOR_H