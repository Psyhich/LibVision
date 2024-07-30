#pragma once

#include "defs.h"

#include "image.h"

namespace vl::filters
{
	void gaussian_filter(Image &image, double standardDeviation, std::size_t kernelSize);
}
