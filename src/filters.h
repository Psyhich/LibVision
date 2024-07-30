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
		Octagon
	};
	std::optional<Shape> to_shape(const std::string &shapeString);

	void gaussian(Image &image, double standardDeviation, std::size_t kernelSize);
	void median(Image &image, std::size_t size, Shape shapeToUse=Shape::Rectangle);
	void truncated_median(Image &image, std::size_t size, std::size_t stdDevCount=2, Shape shapeToUse=Shape::Rectangle);
}
