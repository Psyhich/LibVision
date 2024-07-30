#include "filters.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include <fmt/format.h>
#include <fmt/ranges.h>

namespace vl::filters
{
	void gaussian_filter(Image &image, double standardDeviation, std::size_t kernelSize)
	{
		if (kernelSize % 2 == 0)
		{
			fmt::println("Invalid kernel size: {}, kernel size should be odd number",
				kernelSize);
			return;
		}
		if (image.width <= kernelSize || image.height <= kernelSize)
		{
			fmt::println("Invalid image size: {}x{} to kernel size: {}x{}",
				image.width, image.height, kernelSize, kernelSize);
			return;
		}
		if (image.format != PixelFormat::Grayscale8)
		{
			fmt::println("Unsupported image format");
			return;
		}

		std::vector<double> kernel(kernelSize * kernelSize);
		const std::size_t halfKernel{kernelSize / 2};

		const double inverseDoublePow{1 / (2 * std::pow(standardDeviation, 2))};
		const double inverseDoublePowPi{inverseDoublePow / std::numbers::pi};
		double kernelSum{0};
		for (std::size_t y = 0; y < kernelSize; ++y)
		{
			const int yDisplacement = y - halfKernel;
			const std::size_t yPow = yDisplacement * yDisplacement;
			for (std::size_t x = 0; x < kernelSize; ++x)
			{
				const int xDisplacement = x - halfKernel;
				const std::size_t xPow = xDisplacement * xDisplacement;
				kernel[y * kernelSize + x] = inverseDoublePowPi * std::exp( -((xPow + yPow) * inverseDoublePow));
			}
		}

		const Image imageCopy{image};

		const std::size_t effectiveWidth{image.width - halfKernel};
		const std::size_t effectiveHeight{image.height - halfKernel};
		for (std::size_t y = halfKernel; y < effectiveHeight; ++y)
		{
			for (std::size_t x = halfKernel; x < effectiveWidth; ++x)
			{
				double sum{0};
				for (std::size_t kernelY = 0; kernelY < kernelSize; ++kernelY)
					for (std::size_t kernelX = 0; kernelX < kernelSize; ++kernelX)
						sum += kernel[kernelY * kernelSize + kernelX]
							* (double)imageCopy[x - halfKernel + kernelX, y - halfKernel + kernelY];
				image[x, y] = std::clamp(sum, 0., 256.);
			}
		}
	}
}
