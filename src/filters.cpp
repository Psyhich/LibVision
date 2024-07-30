#include "filters.h"

#include <cassert>
#include <algorithm>
#include <cmath>
#include <vector>
#include <span>

#include <fmt/format.h>
#include <fmt/ranges.h>

std::vector<bool> generate_octagon_mask(std::size_t size)
{
	const std::size_t emptinessSizeSize = std::round(size / 3.);
	assert(emptinessSizeSize * 2 < size);

	const std::size_t sideSize = size - emptinessSizeSize * 2;

	const std::size_t remainingEmptinessIndex{size - emptinessSizeSize - 1};
	std::vector<bool> mask(size * size);
	for (std::size_t y = 0; y < size; ++y)
	{
		for (std::size_t x = 0; x < size; ++x)
		{
			mask[y * size + x] = (x < emptinessSizeSize && y < (emptinessSizeSize - x))
				|| (x >= remainingEmptinessIndex && y < x - remainingEmptinessIndex)
				|| (x < emptinessSizeSize && y > (remainingEmptinessIndex + x))
				|| (x >= remainingEmptinessIndex && y > (size - (x - remainingEmptinessIndex) - 1));
		}
	}

	return mask;
}

namespace vl::filters
{
	std::optional<Shape> to_shape(const std::string &shapeString)
	{
		std::string shapeStringLowCase{shapeString};
		std::transform(begin(shapeStringLowCase), end(shapeStringLowCase),
			begin(shapeStringLowCase), tolower);

		if (shapeStringLowCase == "rectangle")
		{
			return Shape::Rectangle;
		}
		else if (shapeStringLowCase == "octagon")
		{
			return Shape::Octagon;
		}

		return {};
	}

	void gaussian(Image &image, double standardDeviation, std::size_t kernelSize)
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

	void median(Image &image, std::size_t size, Shape shapeToUse)
	{
		if (size % 2 == 0)
		{
			fmt::println("Invalid size of median filter: {}, filter should have odd size", size);
			return;
		}
		if (image.width <= size || image.height <= size)
		{
			fmt::println("Invalid image size: {}x{} to kernel size: {}x{}",
				image.width, image.height, size, size);
			return;
		}
		if (image.format != PixelFormat::Grayscale8)
		{
			fmt::println("Unsupported image format");
			return;
		}

		std::vector<bool> mask;
		switch (shapeToUse)
		{
			case Shape::Rectangle:
				mask.resize(size * size, true);
				break;
			case Shape::Octagon:
				mask = generate_octagon_mask(size);
				break;
		}
		const std::size_t valuesCount = std::count(begin(mask), end(mask), true);


		const std::size_t halfSize{size / 2};
		const std::size_t effectiveWidth{image.width - halfSize};
		const std::size_t effectiveHeight{image.height - halfSize};

		const vl::Image imageCopy{image};
		std::vector<byte> values(valuesCount, 0);
		for (ssize_t y = halfSize; y < effectiveHeight; ++y)
		{
			for (ssize_t x = halfSize; x < effectiveWidth; ++x)
			{
				std::size_t index{0};
				for (ssize_t kernelY = 0; kernelY < size; ++kernelY)
					for (ssize_t kernelX = 0; kernelX < size; ++kernelX)
						if (mask[kernelY * size + kernelX])
							values[index++] = imageCopy[x - halfSize + kernelX, y - halfSize + kernelY];
				assert(index == values.size());

				std::sort(begin(values), end(values));
				image[x, y] = values[values.size() / 2 + 1];
			}
		}
	}
}
