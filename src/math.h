#pragma once

#include "defs.h"

#include "image.h"

namespace vl::math
{
	double entropy(const Image &image);
	double signal_to_noise_ratio(const Image &image);
};
