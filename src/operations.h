#pragma once

#include <fmt/format.h>

#include "image.h"

namespace vl
{
	vl::Image operator+(const vl::Image &lImage, const vl::Image &rImage);
	vl::Image operator-(const vl::Image &lImage, const vl::Image &rImage);
	vl::Image operator/(const vl::Image &lImage, const vl::Image &rImage);
	vl::Image operator*(const vl::Image &lImage, const vl::Image &rImage);

	vl::Image &operator+=(vl::Image &image, const vl::Image &appliedImage);
	vl::Image &operator-=(vl::Image &image, const vl::Image &appliedImage);
	vl::Image &operator/=(vl::Image &image, const vl::Image &appliedImage);
	vl::Image &operator*=(vl::Image &image, const vl::Image &appliedImage);

	namespace impl
	{
		inline void check_for_operation(const vl::Image &lImage, const vl::Image &rImage)
		{
			if (lImage.width != rImage.width
				|| lImage.height != rImage.height)
				throw std::out_of_range{fmt::format("Wrong image size: {}x{} to {}x{}",
					lImage.width, lImage.height, rImage.width, rImage.height)};

			if (lImage.format != rImage.format)
				throw std::logic_error{fmt::format("Wrong image formats specified: {} to {}",
					static_cast<int>(lImage.format), static_cast<int>(rImage.format))};
		}
	}
}
