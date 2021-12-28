#include "pch.h"
#include "Utils.h"

auto GetFilenameFromPath(const std::string& path) -> std::string
{
	auto index = path.rfind('\\');
	if (index == std::string::npos)
	{
		return "";
	}
	return path.substr(index + 1, _MAX_PATH);
}
