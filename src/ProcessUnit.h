#ifndef PROCESS_UNIT_H
#define PROCESS_UNIT_H

#pragma once

#include <string>

struct ProcessUnit
{
	auto Source() const ->std::string { return m_Source; }
	auto Destination() const -> std::string { return m_Destination; }
	auto IsEncode() const -> bool { return m_IsEncode; }

	bool m_IsEncode;
	std::string m_Source;
	std::string m_Destination;
};

#endif // PROCESS_UNIT_H