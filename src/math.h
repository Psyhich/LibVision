#pragma once

#include "defs.h"

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

		inline T operator[](std::size_t row, std::size_t col) const
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

		inline const T *cbegin() const
		{
			return m_data.data();
		}

		inline T *end()
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
};
