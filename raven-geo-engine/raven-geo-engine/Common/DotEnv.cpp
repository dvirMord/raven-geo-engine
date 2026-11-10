#include "DotEnv.hpp"
#include "Constants.hpp"
#include <cstdlib>
#include <fstream>
#include <spdlog/spdlog.h>

namespace Common
{
	// reads KEY=VALUE lines (no spaces, no quotes) into this process's environment, so std::getenv can see them
	void loadDotEnv(const std::string& filePath)
	{
		std::ifstream file(filePath);
		if (!file.is_open())
		{
			spdlog::warn(Constants::Logs::DOTENV_NOT_FOUND, filePath);
			return;
		}

		std::string line;
		while (std::getline(file, line))
		{
			size_t equalsAt = line.find('=');

			// skip empty lines, # comments, and lines without '='
			if (line.empty() || line[0] == '#' || equalsAt == std::string::npos)
			{
				continue;
			}

			std::string key = line.substr(0, equalsAt);
			std::string value = line.substr(equalsAt + 1);

			// for this process only - gone when the engine exits
			_putenv_s(key.c_str(), value.c_str());
		}

		spdlog::info(Constants::Logs::DOTENV_LOADED, filePath);
	}
}
