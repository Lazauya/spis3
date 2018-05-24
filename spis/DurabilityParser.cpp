#include "DurabilityParser.h"

std::unordered_map<std::string, std::pair<Float32, Float32> > spis::durabilityReference;

namespace spis
{
	std::vector<std::string> split(const std::string &s, char delim)
	{
		std::stringstream ss(s);
		std::string item;
		std::vector<std::string> tokens;
		while (std::getline(ss, item, delim))
		{
			tokens.push_back(item);
		}
		return tokens;
	}

	void setupDurabilityReference(std::string refFile)
	{
		std::ifstream d(refFile);
		std::string item;
		while (std::getline(d, item))
		{
			auto tokens = split(item, ';');
			if(tokens.size() == 3)
				durabilityReference[tokens[0]] = std::pair<double, double>(std::stod(tokens[1]), std::stod(tokens[2]));
			}
		}
}