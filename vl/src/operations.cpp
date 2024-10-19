#include "operations.h"


namespace vl
{
	vl::Image operator+(const vl::Image &lImage, const vl::Image &rImage)
	{
		impl::check_for_operation(lImage, rImage);

		auto copy{lImage};
		for (size_t row = 0; row < copy.height(); ++row)
			for (size_t col = 0; col < copy.width(); ++col)
				copy[col, row] += rImage[col, row];

		return copy;
	}

	vl::Image operator-(const vl::Image &lImage, const vl::Image &rImage)
	{
		impl::check_for_operation(lImage, rImage);

		auto copy{lImage};
		for (size_t row = 0; row < copy.height(); ++row)
			for (size_t col = 0; col < copy.width(); ++col)
				copy[col, row] -= rImage[col, row];

		return copy;
	}

	vl::Image operator/(const vl::Image &lImage, const vl::Image &rImage)
	{
		impl::check_for_operation(lImage, rImage);

		auto copy{lImage};
		for (size_t row = 0; row < copy.height(); ++row)
			for (size_t col = 0; col < copy.width(); ++col)
				copy[col, row] /= rImage[col, row];

		return copy;
	}

	vl::Image operator*(const vl::Image &lImage, const vl::Image &rImage)
	{
		impl::check_for_operation(lImage, rImage);

		auto copy{lImage};
		for (size_t row = 0; row < copy.height(); ++row)
			for (size_t col = 0; col < copy.width(); ++col)
				copy[col, row] *= rImage[col, row];

		return copy;
	}


	vl::Image &operator+=(vl::Image &image, const vl::Image &appliedImage)
	{
		impl::check_for_operation(image, appliedImage);

		for (size_t row = 0; row < image.height(); ++row)
			for (size_t col = 0; col < image.width(); ++col)
				image[col, row] += appliedImage[col, row];

		return image;
	}

	vl::Image &operator-=(vl::Image &image, const vl::Image &appliedImage)
	{
		impl::check_for_operation(image, appliedImage);

		for (size_t row = 0; row < image.height(); ++row)
			for (size_t col = 0; col < image.width(); ++col)
				image[col, row] -= appliedImage[col, row];

		return image;
	}

	vl::Image &operator/=(vl::Image &image, const vl::Image &appliedImage)
	{
		impl::check_for_operation(image, appliedImage);

		for (size_t row = 0; row < image.height(); ++row)
			for (size_t col = 0; col < image.width(); ++col)
				image[col, row] /= appliedImage[col, row];

		return image;
	}

	vl::Image &operator*=(vl::Image &image, const vl::Image &appliedImage)
	{
		impl::check_for_operation(image, appliedImage);

		for (size_t row = 0; row < image.height(); ++row)
			for (size_t col = 0; col < image.width(); ++col)
				image[col, row] *= appliedImage[col, row];

		return image;
	}
}
