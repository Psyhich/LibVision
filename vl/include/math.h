#pragma once

#include "defs.h"

#include <cmath>
#include <numeric>

#include "image.h"

namespace vl::math
{
	template <typename T>
	class Matrix
	{
	public:
		Matrix(std::size_t rows, std::size_t cols)
			: m_cols{cols}
			, m_rows{rows}
			, m_data(rows * cols)
		{
		}

		Matrix(std::size_t rank=0)
			: m_cols{rank}
			, m_rows{rank}
			, m_data(rank * rank)
		{
		}

		inline std::size_t rows() const
		{
			return m_rows;
		}

		inline std::size_t cols() const
		{
			return m_cols;
		}

		inline const T &operator[](std::size_t row, std::size_t col) const
		{
			return m_data[row * m_cols + col];
		}

		inline T& operator[](std::size_t row, std::size_t col)
		{
			return m_data[row * m_cols + col];
		}

		inline T *begin()
		{
			return m_data.data();
		}

		inline const T *begin() const
		{
			return m_data.data();
		}

		inline const T *cbegin() const
		{
			return m_data.data();
		}

		inline T *end()
		{
			return m_data.data() + m_data.size();
		}

		inline const T *end() const
		{
			return m_data.data() + m_data.size();
		}

		inline const T *cend() const
		{
			return m_data.data() + m_data.size();
		}

	private:
		std::size_t m_cols;
		std::size_t m_rows;
		std::vector<T> m_data;
	};

	double entropy(const Image &image);
	double signal_to_noise_ratio(const Image &image);

	template<typename Iter>
	std::pair<double, double> get_mean_std_dev(Iter begin, Iter end)
	{
		const std::size_t count{std::distance(begin, end)};
		const double mean = (double)std::accumulate(begin, end, 0) / count;

		double std_dev{0};
		for (auto it = begin; it != end; ++it)
			std_dev += std::pow((double) *it - mean, 2.);
		std_dev = std::sqrt(std_dev / count);

		return {mean, std_dev};
	}
	template<typename ContainerT>
	inline std::pair<double, double> get_mean_std_dev(const ContainerT &container)
	{
		return get_mean_std_dev(container.cbegin(), container.cend());
	}
};
