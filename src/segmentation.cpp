#include "segmentation.h"
#include <algorithm>
#include <set>
#include <ranges>

#include <fmt/format.h>
#include <fmt/ranges.h>

/*
1. Квантизувати тони до приємливої кількості
2. Розрахувати для кожного сектора матрицю сусідніх пікселів по сторонам на кутах: 0°, 45°, 90° та 135° 
	Для цього треба:
	1. Створити матрицю розміром N x N, де N - це максимальне значення тону пікселя
	2. Пройтись по сектору у два ряди та вносити у матрицю у координати (i, j) якщо ми помічаємо 
		квантизоване значення пікселя 'i' по сусідству(0, 45, 90, 135) із значенням 'j'
	
3. Обчислити фічі для кожної матриці(почати із angular second moment, contrast та correlation)
4. Усереднити та закинути у визідну матричку
*/

namespace vl
{
	math::Matrix<SectorParameters> compute_sector_parameters(const Image &image_to_compute)
	{
		impl::QuantizedTonesLookupTable table;
		const std::size_t sector_size{table.quantize(image_to_compute, 64)};
		fmt::println("Calculated sector size: {}", sector_size);
		if (sector_size <= 1)
		{
			fmt::println("Sector is too small: {}, leaving", sector_size);
			return {};
		}

		if (sector_size >= std::max(image_to_compute.height, image_to_compute.width))
		{
			fmt::println("Sector to large for image: {}x{} sector size: {}", image_to_compute.width, image_to_compute.height, sector_size);
			return {};
		}

		const std::size_t sectors_in_width{image_to_compute.width / sector_size};
		const std::size_t sectors_in_height{image_to_compute.height / sector_size};

		math::Matrix<std::size_t> angle_0_neighbours{sector_size};
		math::Matrix<std::size_t> angle_45_neighbours{sector_size};
		math::Matrix<std::size_t> angle_90_neighbours{sector_size};
		math::Matrix<std::size_t> angle_135_neighbours{sector_size};

		const auto quantized{
			[&] (std::size_t row, std::size_t col)
			{
				return table[image_to_compute[col, row]];
			}
		};

		const std::size_t normalizing_constant{impl::calculate_normalizing_constant(sector_size)};
		math::Matrix<SectorParameters> params{sectors_in_height, sectors_in_width};

		for (std::size_t height = 0; height < sectors_in_height; ++height)
		{
			const std::size_t sectorStartY{height * sector_size};
			for (std::size_t width = 0; width < sectors_in_width; ++width)
			{
				std::ranges::fill(angle_0_neighbours, 0);
				std::ranges::fill(angle_45_neighbours, 0);
				std::ranges::fill(angle_90_neighbours, 0);
				std::ranges::fill(angle_135_neighbours, 0);

				const std::size_t sectorStartX{width * sector_size};
				for (std::size_t i = 0; i < sector_size; ++i)
				{
					for (std::size_t j = 0; j < sector_size; ++j)
					{
						const std::size_t current_quantized{quantized(sectorStartY + i, sectorStartX + j)};

						// 0
						if (j < sector_size - 1)
						{
							++angle_0_neighbours[current_quantized,
								quantized(sectorStartY + i, sectorStartX + j + 1)];
						}
						if (j > 0)
						{
							++angle_0_neighbours[current_quantized,
								quantized(sectorStartY + i, sectorStartX + j - 1)];
						}

						// 90
						if (i < sector_size - 1)
						{
							++angle_90_neighbours[current_quantized,
								quantized(sectorStartY + i + 1, sectorStartX + j)];
						}
						if (i > 0)
						{
							++angle_90_neighbours[current_quantized,
								quantized(sectorStartY + i - 1, sectorStartX + j)];
						}

						// 45
						if (i < sector_size - 1 && j > 0)
						{
							++angle_45_neighbours[current_quantized,
								quantized(sectorStartY + i + 1, sectorStartX + j - 1)];
						}
						if (i > 0 && j < sector_size - 1)
						{
							++angle_45_neighbours[current_quantized,
								quantized(sectorStartY + i - 1, sectorStartX + j + 1)];
						}

						// 135
						if (i < sector_size - 1 && j < sector_size - 1)
						{
							++angle_135_neighbours[current_quantized,
								quantized(sectorStartY + i + 1, sectorStartX + j + 1)];
						}
						if (i > 0 && j > 0)
						{
							++angle_135_neighbours[current_quantized,
								quantized(sectorStartY + i - 1, sectorStartX + j - 1)];
						}
					}
				}

				params[height, width].angular_second_moment =
					(impl::calculate_angular_second_moment(angle_0_neighbours, normalizing_constant)
						+ impl::calculate_angular_second_moment(angle_45_neighbours, normalizing_constant)
						+ impl::calculate_angular_second_moment(angle_90_neighbours, normalizing_constant)
						+ impl::calculate_angular_second_moment(angle_135_neighbours, normalizing_constant))
					/ 4;
			}
		}

		return params;
	}

	namespace impl
	{
		std::size_t calculate_normalizing_constant(std::size_t sector_size)
		{
			return 4 * sector_size * (sector_size - 1) // Horizontal + vertical
				+ 4 * std::pow(sector_size - 1, 2); // Diagonals
		}

		double calculate_angular_second_moment(const math::Matrix<std::size_t> &occurance_matrix, std::size_t normalizing_constant)
		{
			double sum = 0;
			for (std::size_t row = 0; row < occurance_matrix.rows(); ++row)
				for (std::size_t col = 0; col < occurance_matrix.cols(); ++col)
					sum += std::pow((double)occurance_matrix[row, col] / normalizing_constant, 2);

			return sum;
		}

		std::size_t QuantizedTonesLookupTable::quantize(const vl::Image &image, std::size_t desired_count)
		{
			std::set<byte> tones;
			std::ranges::copy(image, std::inserter(tones, begin(tones)));

			if (tones.size() <= desired_count)
			{
				for (byte tone : tones)
					m_table.emplace(tone, tone);
				return desired_count;
			}
			const byte min_value{*std::ranges::min_element(tones)};

			const double coefficient = (double)desired_count / tones.size();
			for (byte tone : tones)
				m_table.emplace(tone, (byte) (tone - min_value) * coefficient);

			return desired_count;
		}
	}
}
