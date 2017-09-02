#pragma once

#include <string>

struct TRomImage;
class GageMemory;

namespace TRomImageLoader
{
	void load(TRomImage& image, GageMemory& mem, std::string& lib_folder_path);
	bool parse(std::string& path, TRomImage& image);
	void parseHeader(TRomImage& image);
	void parseExportDir(TRomImage& image);
	void checkHeaderValidity(TRomImage& image);
};