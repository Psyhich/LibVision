#pragma once

#include "defs.h"

#include <optional>
#include <string>

#include "image.h"

namespace vl::filters
{
	enum class Shape
	{
		Rectangle,
		Circle,
		Octagon
	};
	std::optional<Shape> to_shape(const std::string &shapeString);

	void gaussian(Image &image, double standardDeviation, std::size_t kernelSize);

	void median(Image &image, std::size_t size, Shape shapeToUse=Shape::Rectangle);
	void truncated_median(Image &image, std::size_t size, std::size_t stdDevCount=2, Shape shapeToUse=Shape::Rectangle);
	void hybrid_median(Image &image, std::size_t size);

	void erosion(Image &image, Shape shape, std::size_t size);
	void dilation(Image &image, Shape shape, std::size_t size);

	void top_hat(Image &image, int innerRadius, int outterRadius, std::size_t threshold, bool dark=true);

	namespace impl
	{
		std::vector<bool> create_mask(std::size_t size, Shape shape);
		std::size_t get_mask_pixels_count(const std::vector<bool> &mask);

		std::vector<bool> generate_octagon_mask(std::size_t size);
		std::vector<bool> generate_circle_mask(std::size_t size);
	}
}
