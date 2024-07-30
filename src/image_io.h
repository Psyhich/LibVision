#pragma once

#include "defs.h"

#include <expected>
#include <string>

#include "image.h"

namespace vl::ImageIO
{
	enum class ErrorType
	{
		IOError,
		FormatError
	};

	struct ReadError
	{
		ErrorType type;
		std::string description;
	};
	struct WriteError
	{
		ErrorType type;
		std::string description;
	};

	std::expected<vl::Image, ReadError> read_png(const std::string &path);
	std::expected<void, WriteError> write_png(const vl::Image &imageToWrite, const std::string &path);
}
