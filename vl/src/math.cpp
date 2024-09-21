#include "math.h"

#include <fmt/format.h>

namespace vl::math
{
	double entropy(const Image &image)
	{
		if (image.format != PixelFormat::Grayscale8)
		{
			fmt::println("Non grayscale formats are not supported");
			return 0;
		}
		std::array<std::size_t, 256> frequencies{};
		for (std::size_t y = 0; y < image.height; ++y)
			for (std::size_t x = 0; x < image.width; ++x)
				++frequencies[image[x, y]];
		double entropy{0};
		const double size = image.width * image.height; 
		for (std::size_t i = 0; i < frequencies.size(); ++i)
		{
			if (frequencies[i] == 0) continue;

			const double possibility{frequencies[i] / size};
			entropy += possibility * std::log2(possibility);
		}

		return -entropy;
	}

	double signal_to_noise_ratio(const Image &image)
	{
		if (image.format != PixelFormat::Grayscale8)
		{
			fmt::println("Non grayscale formats are not supported");
			return 0;
		}
		std::array<std::size_t, 256> frequencies{};
		for (std::size_t y = 0; y < image.height; ++y)
			for (std::size_t x = 0; x < image.width; ++x)
				++frequencies[image[x, y]];

		std::array<double, 256> possibilities{};
		const double pixelsCount = image.width * image.height; 
		for (std::size_t i = 0; i < possibilities.size(); ++i)
		{
			if (frequencies[i] == 0) continue;

			possibilities[i] = frequencies[i] / pixelsCount;
		}

		double mean{};
		for (std::size_t i = 0; i < possibilities.size(); ++i)
			mean += possibilities[i] * i;

		double stdDev{};
		for (std::size_t i = 0; i < possibilities.size(); ++i)
			stdDev += possibilities[i] * std::pow(i - mean, 2);
		stdDev = std::sqrt(stdDev);

		return mean / stdDev;
	}
}
