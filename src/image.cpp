#include "image.h"

namespace vl
{
	std::size_t to_pixel_size(PixelFormat format)
	{
		switch (format)
		{
			case PixelFormat::Grayscale8:
				return 1;
			default:
				return 0;
		}
	}

	Image::Image(const std::span<byte> &bytes, std::size_t _width, std::size_t _height, PixelFormat _format)
		: m_rawBytes{begin(bytes), end(bytes)}
		, width{_width}
		, height{_height}
		, format{_format}
	{
	}

	Image::Image(std::vector<byte> &&bytes, std::size_t _width, std::size_t _height, PixelFormat _format)
		: m_rawBytes{std::move(bytes)}
		, width{_width}
		, height{_height}
		, format{_format}
	{
	}
}
