#pragma once

#include "defs.h"

#include <span>
#include <vector>

namespace vl
{
	enum class PixelFormat
	{
		Grayscale8,
	};
	std::size_t to_pixel_size(PixelFormat format);

	class Image
	{
	public:
		Image(const std::span<byte> &bytes, std::size_t width, std::size_t height, PixelFormat format);
		Image(std::vector<byte> &&bytes, std::size_t width, std::size_t height, PixelFormat format);

		inline byte &operator[](std::size_t x, std::size_t y)
		{
			return m_rawBytes[y * width + x];
		}
		inline const byte &operator[](std::size_t x, std::size_t y) const
		{
			return m_rawBytes[y * width + x];
		}

		const PixelFormat format;
		const std::size_t width;
		const std::size_t height;

	private:
		std::vector<byte> m_rawBytes;
	};

}
