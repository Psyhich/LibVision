#include "filters.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <span>
#include <vector>

#include <fmt/format.h>
#include <fmt/ranges.h>

namespace vl::filters
{
	std::optional<Shape> to_shape(const std::string &shapeString)
	{
		std::string shapeStringLowCase{shapeString};
		std::transform(begin(shapeStringLowCase), end(shapeStringLowCase),
			begin(shapeStringLowCase), tolower);

		if (shapeStringLowCase == "rectangle")
			return Shape::Rectangle;
		else if (shapeStringLowCase == "octagon")
			return Shape::Octagon;
		else if (shapeStringLowCase == "circle")
			return Shape::Circle;

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
		if (image.width() <= kernelSize || image.height() <= kernelSize)
		{
			fmt::println("Invalid image size: {}x{} to kernel size: {}x{}",
				image.width(), image.height(), kernelSize, kernelSize);
			return;
		}
		if (image.format() != PixelFormat::Grayscale8)
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

		const std::size_t effectiveWidth{image.width() - halfKernel};
		const std::size_t effectiveHeight{image.height() - halfKernel};
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
		if (image.width() <= size || image.height() <= size)
		{
			fmt::println("Invalid image size: {}x{} to kernel size: {}x{}",
				image.width(), image.height(), size, size);
			return;
		}
		if (image.format() != PixelFormat::Grayscale8)
		{
			fmt::println("Unsupported image format");
			return;
		}

		const std::vector<bool> mask{impl::create_mask(size, shapeToUse)};
		const std::size_t valuesCount = impl::get_mask_pixels_count(mask);

		const std::size_t halfSize{size / 2};
		const std::size_t effectiveWidth{image.width() - halfSize};
		const std::size_t effectiveHeight{image.height() - halfSize};

		const vl::Image imageCopy{image};
		std::vector<byte> values(valuesCount, 0);
		for (std::size_t y = halfSize; y < effectiveHeight; ++y)
		{
			for (std::size_t x = halfSize; x < effectiveWidth; ++x)
			{
				std::size_t index{0};
				for (std::size_t kernelY = 0; kernelY < size; ++kernelY)
					for (std::size_t kernelX = 0; kernelX < size; ++kernelX)
						if (mask[kernelY * size + kernelX])
							values[index++] = imageCopy[x - halfSize + kernelX, y - halfSize + kernelY];
				assert(index == values.size());

				std::ranges::sort(values);
				image[x, y] = values[values.size() / 2 + 1];
			}
		}
	}

	void truncated_median(Image &image, std::size_t size, std::size_t stdDevCount, Shape shapeToUse)
	{
		if (size % 2 == 0)
		{
			fmt::println("Invalid size of truncated median filter: {}, filter should have odd size", size);
			return;
		}
		if (image.width() <= size || image.height() <= size)
		{
			fmt::println("Invalid image size: {}x{} to kernel size: {}x{}",
				image.width(), image.height(), size, size);
			return;
		}
		if (image.format() != PixelFormat::Grayscale8)
		{
			fmt::println("Unsupported image format");
			return;
		}

		std::vector<bool> mask {impl::create_mask(size, shapeToUse)};
		const std::size_t valuesCount = std::count(begin(mask), end(mask), true);

		const std::size_t halfSize{size / 2};
		const std::size_t effectiveWidth{image.width() - halfSize};
		const std::size_t effectiveHeight{image.height() - halfSize};

		const vl::Image imageCopy{image};
		std::vector<byte> values(valuesCount, 0);
		std::array<int, 256> frequencies{};

		std::size_t index{0};
		std::size_t mean{0};
		std::size_t powMean{0};
		for (std::size_t y = halfSize; y < effectiveHeight; ++y)
		{
			for (std::size_t x = halfSize; x < effectiveWidth; ++x)
			{
				index = 0;

				for (std::size_t kernelY = 0; kernelY < size; ++kernelY)
					for (std::size_t kernelX = 0; kernelX < size; ++kernelX)
						if (mask[kernelY * size + kernelX])
							values[index++] = imageCopy[x - halfSize + kernelX, y - halfSize + kernelY];

				assert(index == values.size());

				for (std::size_t i = 0; i < values.size(); ++i)
					++frequencies[values[i]];

				for (std::size_t i = 0; i < values.size(); ++i)
				{
					mean += frequencies[values[i]] / (double)values.size() * values[i];
					powMean += std::pow(frequencies[values[i]] / (double)values.size() * values[i], 2);
				}
				mean /= values.size();
				powMean /= values.size();
				const double threshold{std::sqrt(powMean - mean * mean) * stdDevCount};

				std::ranges::sort(values);

				auto acceptableStart{std::upper_bound(begin(values), end(values),
					mean - threshold)
				};
				const auto acceptableEnd{std::lower_bound(begin(values), end(values),
					mean + threshold)
				};

				if (acceptableStart == end(values))
					acceptableStart = begin(values);

				image[x, y] = *(acceptableStart + std::distance(acceptableStart, acceptableEnd) / 2 + 1);
				std::memset(frequencies.data(), 0, frequencies.size());
			}
		}
	}

	void hybrid_median(Image &image, std::size_t size)
	{
		if (size % 2 == 0)
		{
			fmt::println("Invalid size of hybrid median filter: {}, filter should have odd size", size);
			return;
		}
		if (image.width() <= size || image.height() <= size)
		{
			fmt::println("Invalid image size: {}x{} to kernel size: {}x{}",
				image.width(), image.height(), size, size);
			return;
		}
		if (image.format() != PixelFormat::Grayscale8)
		{
			fmt::println("Unsupported image format");
			return;
		}

		const std::size_t halfSize{size / 2};
		const std::size_t effectiveWidth{image.width() - halfSize};
		const std::size_t effectiveHeight{image.height() - halfSize};


		const Image copy{image};
		std::vector<byte> pixelsToCheck(size * 2 - 1);
		std::array<byte, 3> medians;
		for (std::size_t y = halfSize; y < effectiveHeight; ++y)
		{
			for (std::size_t x = halfSize; x < effectiveWidth; ++x)
			{
				for (std::size_t i = 0; i < size; ++i)
				{
					pixelsToCheck[i] = copy[x - halfSize + i, y - halfSize + i];
					if (i != halfSize + 1)
						pixelsToCheck[size + i - (i > halfSize + 1)] = copy[x + halfSize - i, y + halfSize - i];
				}
				std::ranges::sort(pixelsToCheck);

				medians[0] = pixelsToCheck[pixelsToCheck.size() / 2 + 1];
				medians[1] = copy[x, y];

				for (std::size_t i = 0; i < size; ++i)
				{
					pixelsToCheck[i] = copy[x, y - halfSize + i];
					if (i != halfSize + 1)
						pixelsToCheck[size + i - (i > halfSize + 1)] = copy[x + halfSize - i, y];
				}
				std::ranges::sort(pixelsToCheck);
				medians[2] = pixelsToCheck[pixelsToCheck.size() / 2 + 1];

				std::ranges::sort(medians);
				image[x, y] = medians[1];
			}
		}
	}

	void erosion(Image &image, Shape shape, std::size_t size)
	{
		if (size % 2 == 0)
		{
			fmt::println("Invalid size of erosion filter: {}, filter should have odd size", size);
			return;
		}
		if (image.width() <= size || image.height() <= size)
		{
			fmt::println("Invalid image size: {}x{} to kernel size: {}x{}",
				image.width(), image.height(), size, size);
			return;
		}
		if (image.format() != PixelFormat::Grayscale8)
		{
			fmt::println("Unsupported image format");
			return;
		}

		const std::size_t halfSize{size / 2};
		const std::size_t effectiveWidth{image.width() - halfSize};
		const std::size_t effectiveHeight{image.height() - halfSize};

		const auto mask{impl::create_mask(size, shape)};
		const Image copy{image};
		for (std::size_t y = halfSize; y < effectiveHeight; ++y)
		{
			for (std::size_t x = halfSize; x < effectiveWidth; ++x)
			{
				byte min = copy[x, y];
				for (std::size_t i = 0; i < size; ++i)
					for (std::size_t j = 0; j < size; ++j)
						if (mask[i * size + j])
							min = std::min(copy[x - halfSize + i, y - halfSize + j], min);

				image[x, y] = min;
			}
		}
	}

	void dilation(Image &image, Shape shape, std::size_t size)
	{
		if (size % 2 == 0)
		{
			fmt::println("Invalid size of dilation filter: {}, filter should have odd size", size);
			return;
		}
		if (image.width() <= size || image.height() <= size)
		{
			fmt::println("Invalid image size: {}x{} to kernel size: {}x{}",
				image.width(), image.height(), size, size);
			return;
		}
		if (image.format() != PixelFormat::Grayscale8)
		{
			fmt::println("Unsupported image format");
			return;
		}

		const std::size_t halfSize{size / 2};
		const std::size_t effectiveWidth{image.width() - halfSize};
		const std::size_t effectiveHeight{image.height() - halfSize};

		const auto mask{impl::create_mask(size, shape)};
		const Image copy{image};
		for (std::size_t y = halfSize; y < effectiveHeight; ++y)
		{
			for (std::size_t x = halfSize; x < effectiveWidth; ++x)
			{
				byte max = copy[x, y];
				for (std::size_t i = 0; i < size; ++i)
					for (std::size_t j = 0; j < size; ++j)
						if (mask[i * size + j])
							max = std::max(copy[x - halfSize + i, y - halfSize + j], max);

				image[x, y] = max;
			}
		}
	}

	void top_hat(Image &image, int innerRadius, int outterRadius, std::size_t threshold, bool dark)
	{
		if (innerRadius % 2 == 0)
		{
			fmt::println("Invalid inner radius of top-hat filter: {}, filter should have odd size", innerRadius);
			return;
		}
		if (outterRadius % 2 == 0)
		{
			fmt::println("Outter inner radius of top-hat filter: {}, filter should have odd size", outterRadius);
			return;
		}

		if (image.width() <= outterRadius || image.height() <= outterRadius)
		{
			fmt::println("Invalid image size: {}x{} to outter size: {}",
				image.width(), image.height(), outterRadius);
			return;
		}
		if (image.format() != PixelFormat::Grayscale8)
		{
			fmt::println("Unsupported image format");
			return;
		}

		const std::size_t halfSize{(std::size_t)outterRadius / 2};
		const std::size_t effectiveWidth{image.width() - halfSize};
		const std::size_t effectiveHeight{image.height() - halfSize};

		const auto innerMask{impl::create_mask(outterRadius, innerRadius, Shape::Circle)};
		
		const Image original{image};

		for (std::size_t y = halfSize; y < effectiveHeight; ++y)
			for (std::size_t x = halfSize; x < effectiveWidth; ++x)
			{
				int maxOutter = -1;
				int maxInner = -1;

				for (std::size_t i = 0; i < outterRadius; ++i)
					for (std::size_t j = 0; j < outterRadius; ++j)
					{
						if (innerMask[i * outterRadius + j])
							maxInner = std::max((int)original[x - halfSize + i, y - halfSize + j], maxInner);
						else
							maxOutter = std::max((int)original[x - halfSize + i, y - halfSize + j], maxOutter);
					}

				image[x, y] = std::abs(maxOutter - maxInner) >= threshold ? original[x, y] : !dark * 255;
			}
	}

	void rolling_ball(Image &image, int innerRadius, int outterRadius, std::size_t threshold, bool dark)
	{
		if (innerRadius % 2 == 0)
		{
			fmt::println("Invalid inner radius of rolling ball filter: {}, filter should have odd size", innerRadius);
			return;
		}
		if (outterRadius % 2 == 0)
		{
			fmt::println("Outter inner radius of rolling ball filter: {}, filter should have odd size", outterRadius);
			return;
		}

		if (image.width() <= outterRadius || image.height() <= outterRadius)
		{
			fmt::println("Invalid image size: {}x{} to outter size: {}",
				image.width(), image.height(), outterRadius);
			return;
		}
		if (image.format() != PixelFormat::Grayscale8)
		{
			fmt::println("Unsupported image format");
			return;
		}

		const auto innerMask{impl::create_mask(outterRadius, innerRadius, Shape::Circle)};

		const int regionSize{outterRadius * 2};
		const int halfSize{outterRadius};
		const std::size_t effectiveWidth{image.width() - halfSize};
		const std::size_t effectiveHeight{image.height() - halfSize};

		int innerMin;
		int outterMin;
		const Image copy{image};
		for (std::size_t row = halfSize; row < effectiveHeight; ++row)
		{
			for (std::size_t col = halfSize; col < effectiveWidth; ++col)
			{
				innerMin = std::numeric_limits<byte>::max();
				outterMin = std::numeric_limits<byte>::max();

				for (std::size_t regionRow = 0; regionRow < regionSize; ++regionRow)
					for (std::size_t regionCol = 0; regionCol < regionSize; ++regionCol)
					{
						if (innerMask[regionRow * outterRadius + regionCol])
							innerMin = std::min((int)copy[col - halfSize + regionCol, row - halfSize + regionRow], innerMin);
						else
							outterMin = std::min((int)copy[col - halfSize + regionCol, row - halfSize + regionRow], outterMin);
					}

				image[col, row] = std::abs(innerMin - outterMin) > threshold ? image[col, row] : !dark * 255;
			}
		}
	}

	namespace impl
	{
		std::vector<bool> create_mask(std::size_t size, Shape shape)
		{
			return create_mask(size, size, shape);
		}

		std::vector<bool> create_mask(std::size_t size, std::size_t shapeSize, Shape shape)
		{
			assert (shapeSize <= size);
			switch (shape)
			{
				case Shape::Rectangle: return generate_rectangle_mask(size, shapeSize);
				case Shape::Circle: return impl::generate_circle_mask(size, shapeSize);
				case Shape::Octagon: return generate_octagon_mask(size, shapeSize);
			}
			return {};
		}

		std::size_t get_mask_pixels_count(const std::vector<bool> &mask)
		{
			return std::ranges::count(mask, true);
		}

		std::vector<bool> generate_rectangle_mask(std::size_t size, std::size_t shapeSize)
		{
			std::vector<bool> mask(size * size);
			const std::size_t displacement{(size - shapeSize) / 2};

			for (std::size_t y = 0; y < shapeSize; ++y)
				for (std::size_t x = 0; x < shapeSize; ++x)
					mask[(y + displacement) * size + x + displacement] = 1;

			return mask;
		}

		std::vector<bool> generate_octagon_mask(std::size_t size, std::size_t shapeSize)
		{
			const std::size_t emptinessSize = std::round(shapeSize / 3.);
			assert(emptinessSize * 2 < size);

			const std::size_t sideSize = shapeSize - emptinessSize * 2;

			const std::size_t remainingEmptinessIndex{shapeSize - emptinessSize - 1};
			std::vector<bool> mask(size * size);
			const std::size_t displacement{(size - shapeSize) / 2};
			for (std::size_t y = 0; y < shapeSize; ++y)
				for (std::size_t x = 0; x < shapeSize; ++x)
				{
					mask[(y + displacement) * size + x + displacement] = (x < emptinessSize && y < (emptinessSize - x))
						|| (x >= remainingEmptinessIndex && y < x - remainingEmptinessIndex)
						|| (x < emptinessSize && y > (remainingEmptinessIndex + x))
						|| (x >= remainingEmptinessIndex && y > (shapeSize - (x - remainingEmptinessIndex) - 1));
				}

			return mask;
		}

		std::vector<bool> generate_circle_mask(std::size_t size, std::size_t shapeSize)
		{
			std::vector<bool> mask(size * size);
			const std::size_t displacement{(size - shapeSize) / 2};

			const std::size_t halfSize = (shapeSize / 2) * (shapeSize / 2);
			for (std::size_t i = 0; i < shapeSize; ++i)
				for (std::size_t j = 0; j < shapeSize; ++j)
					mask[(i + displacement) * size + j + displacement] =
						halfSize >= std::pow((i - halfSize), 2) + std::pow((j - halfSize), 2);

			return mask;
		}
	}
}
