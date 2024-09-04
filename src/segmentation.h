#pragma once

#include <cassert>
#include <fmt/format.h>

#include <vector>
#include <map>

#include "image.h"
#include "math.h"

template<typename T>
void print(const vl::math::Matrix<T> &matr)
{
	for (std::size_t row = 0; row < matr.rows(); ++row)
	{
		for (std::size_t col = 0; col < matr.cols(); ++col)
			fmt::print("{} ", matr[row, col]);

		fmt::print("\n");
	}
}

namespace vl
{
	struct SectorParameters
	{
		double angular_second_moment;
		double contrast;
		double correlation;
	};

	math::Matrix<SectorParameters> compute_sector_parameters(const Image &imageToCompute);

	namespace impl
	{
		std::size_t calculate_normalizing_constant(std::size_t sector_size);
		double calculate_angular_second_moment(const math::Matrix<std::size_t> &occurance_matrix, std::size_t normalizing_constant);

		class QuantizedTonesLookupTable
		{
		public:
			std::size_t quantize(const vl::Image &image, std::size_t desired_count);

			inline byte operator[](byte tone) const
			{
				assert(m_table.contains(tone));
				return m_table.at(tone);
			}

		private:
			std::map<int, int> m_table;
		};
	}
}
