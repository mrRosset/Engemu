#pragma once

#include <string>

struct TRomImage;
class Memory_Interface;

namespace TRomImageLoader
{
	void load(TRomImage& image, Memory_Interface& mem, std::string& lib_folder_path);
	bool parse(std::string& path, TRomImage& image);
	void parseHeader(TRomImage& image);
	void parseExportDir(TRomImage& image);
	void checkHeaderValidity(TRomImage& image);
};