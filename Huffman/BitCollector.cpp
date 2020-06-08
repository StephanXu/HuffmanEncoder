#include "pch.h"
#include "BitCollector.h"

auto BitCollector::Unpacked() const noexcept -> UnitType { return m_Unpacked; }

auto BitCollector::RedundancyBit() const noexcept -> size_t { return m_RedundancyBit; }

auto BitCollector::PushImpl(const UnitType value, const size_t beginPos, const size_t endPos) -> void
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
