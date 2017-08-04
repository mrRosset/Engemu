#include <experimental/filesystem>
#include <unordered_map>
#include <fstream>
#include "SymbolsManager.h"


static std::unordered_map<u32, std::string> symbols;

void loadFile(std::string& symbol_file) {
	std::ifstream infile(symbol_file);
	std::string line;
	while (std::getline(infile, line))
	{
		size_t found = line.find(": ");
		if (found != std::string::npos)
		{
			std::string addr_s = line.substr(0, found);
			std::string symbol = line.substr(found + 2, std::string::npos);
			u32 addr;
			sscanf(addr_s.c_str(), "%x", &addr);
			symbols[addr] = symbol;
		}

	}
}

void Symbols::load(std::string& symbols_folder_path) {
	for (auto & p : std::experimental::filesystem::directory_iterator(symbols_folder_path)) {
		loadFile(p.path().string());
	}
}

bool Symbols::hasFunctionName(u32 address) {
	return symbols.find(address) != symbols.end();
}

std::string Symbols::getFunctionName(u32 address) {
	return symbols[address];
}

std::string Symbols::getFunctionNameOrElse(u32 address) {
	if (hasFunctionName(address)) {
		return getFunctionName(address);
	}
	else {
		return std::to_string(address);
	}
}
