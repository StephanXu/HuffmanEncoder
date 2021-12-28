#pragma once
#ifndef BIT_COLLECTOR_H
#define BIT_COLLECTOR_H

#include <vector>

/// <summary>
/// Generate a buffer from bit.
/// </summary>
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

	auto Unpacked() const noexcept -> UnitType;

	auto RedundancyBit() const noexcept -> size_t;

private:
	UnitType m_Unpacked{};
	size_t m_RedundancyBit{};
	std::vector<UnitType>& m_Buffer;

	auto PushImpl(const UnitType value, const size_t beginPos, const size_t endPos) -> void;
};

#endif // BIT_COLLECTOR_H