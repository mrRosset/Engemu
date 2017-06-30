#pragma once

#include <string>

struct TRomImage;

namespace TRomImageLoader
{
	bool load(std::string path, TRomImage& image);
	void parseHeader(TRomImage& image);
	void checkHeaderValidity(TRomImage& image);
};