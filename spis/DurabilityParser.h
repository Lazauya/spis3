#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <unordered_map>

namespace spis
{
	extern std::unordered_map<std::string, std::pair<Float32, Float32> > durabilityReference;

	std::vector<std::string> split(const std::string &s, char delim);

	void setupDurabilityReference(std::string refFile);
}