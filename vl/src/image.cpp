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
		: m_rawBytes{bytes.begin(), bytes.end()}
		, m_width{_width}
		, m_height{_height}
		, m_format{_format}
	{
	}

	Image::Image(std::vector<byte> &&bytes, std::size_t _width, std::size_t _height, PixelFormat _format)
		: m_rawBytes{std::move(bytes)}
		, m_width{_width}
		, m_height{_height}
		, m_format{_format}
	{
	}

	Image::Image(std::size_t _width, std::size_t _height, PixelFormat _format)
		: m_rawBytes(_width * _height, 0)
		, m_width{_width}
		, m_height{_height}
		, m_format{_format}
	{
	}
}
