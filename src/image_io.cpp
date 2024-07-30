#include "image_io.h"

#include <memory>

#include <fmt/format.h>

#include <png.h>

struct FCloseDeleter
{
	void operator()(FILE *file)
	{
		fclose(file);
	}
};

using PFILE = std::unique_ptr<FILE, FCloseDeleter>;

struct InfoReadStructPair
{
	png_structp png_ptr{nullptr};
	png_infop info_ptr{nullptr};
	png_infop end_info{nullptr};

	~InfoReadStructPair()
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	}
};

struct InfoWriteStructPair
{
	png_structp png_ptr{nullptr};
	png_infop info_ptr{nullptr};

	~InfoWriteStructPair()
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
	}
};

void user_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
	fmt::println("LibPNG error: {}", error_msg);
}
void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	fmt::println("LibPNG warning: {}", warning_msg);
}

namespace vl::ImageIO
{
	std::expected<vl::Image, ReadError> read_png(const std::string &path)
	{
		PFILE readFile{fopen(path.c_str(), "rb")};
		if (readFile == nullptr)
		{
			return std::unexpected<ReadError>({ErrorType::IOError,
				fmt::format("Failed to read {}: {}", path, std::strerror(errno))
			});
		}

		std::array<byte, 8> header{};
		if (fread(header.data(), 1, header.size(), readFile.get()) != header.size())
		{
			if (std::feof(readFile.get()))
			{
				return std::unexpected<ReadError>({ErrorType::FormatError,
					fmt::format("Failed to read {}: This is not a PNG file(end of file reached unexpectedly)", path)
				});
			}
			if (std::ferror(readFile.get()))
			{
				return std::unexpected<ReadError>({ErrorType::IOError,
					fmt::format("Failed to read {}: {}", path, std::strerror(errno))
				});
			}
		}
		if (png_sig_cmp(header.data(), 0, header.size()))
		{
			return std::unexpected<ReadError>({ErrorType::FormatError,
				fmt::format("Failed to read {}: This is not a PNG file(header check)", path)
			});
		}


		InfoReadStructPair infoStructPair{};
		infoStructPair.png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
			nullptr, user_error_fn, user_warning_fn);
		if (infoStructPair.png_ptr == nullptr)
		{
			return std::unexpected<ReadError>({ErrorType::IOError,
				fmt::format("Failed to read {}: Failed to create png_struct for reading", path)
			});
		}

		infoStructPair.info_ptr = png_create_info_struct(infoStructPair.png_ptr);
		if (infoStructPair.info_ptr == nullptr)
		{
			return std::unexpected<ReadError>({ErrorType::IOError,
				fmt::format("Failed to read {}: Failed to create png_info for reading", path)
			});
		}

		png_init_io(infoStructPair.png_ptr, readFile.get());

		png_set_sig_bytes(infoStructPair.png_ptr, header.size());
		png_read_info(infoStructPair.png_ptr, infoStructPair.info_ptr);

		png_set_interlace_handling(infoStructPair.png_ptr);

		png_uint_32 width{png_get_image_width(infoStructPair.png_ptr, infoStructPair.info_ptr)};
		png_uint_32 height{png_get_image_height(infoStructPair.png_ptr, infoStructPair.info_ptr)};
		int colorType{png_get_color_type(infoStructPair.png_ptr, infoStructPair.info_ptr)};
		int bitDepth{png_get_bit_depth(infoStructPair.png_ptr, infoStructPair.info_ptr)};

		if (colorType == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(infoStructPair.png_ptr);

		if (colorType & PNG_COLOR_MASK_ALPHA)
			png_set_strip_alpha(infoStructPair.png_ptr);

		if (colorType == PNG_COLOR_TYPE_RGB)
			png_set_rgb_to_gray(infoStructPair.png_ptr, 1, -1, -1);

		if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
			png_set_expand_gray_1_2_4_to_8(infoStructPair.png_ptr);

		png_read_update_info(infoStructPair.png_ptr, infoStructPair.info_ptr);

		width = png_get_image_width(infoStructPair.png_ptr, infoStructPair.info_ptr);
		height = png_get_image_height(infoStructPair.png_ptr, infoStructPair.info_ptr);
		colorType = png_get_color_type(infoStructPair.png_ptr, infoStructPair.info_ptr);
		bitDepth = png_get_bit_depth(infoStructPair.png_ptr, infoStructPair.info_ptr);

		const std::size_t rowBytes{png_get_rowbytes(infoStructPair.png_ptr, infoStructPair.info_ptr)};
		std::vector<byte> bytes(rowBytes * height);

		std::vector<byte *> rows(height);
		rows[0] = &bytes[0];
		for (std::size_t i = 1; i < height; ++i)
		{
			rows[i] = rows[i - 1] + rowBytes;
		}

		png_read_image(infoStructPair.png_ptr, rows.data());

		return vl::Image{bytes, width, height, PixelFormat::Grayscale8};
	}

	std::expected<void, WriteError> write_png(const Image &image, const std::string &path)
	{
		PFILE readFile{fopen(path.c_str(), "wb")};
		if (readFile == nullptr)
		{
			return std::unexpected<WriteError>({ErrorType::IOError,
				fmt::format("Failed writing to {}: {}", path, std::strerror(errno))
			});
		}

		InfoWriteStructPair infoStructPair{};
		infoStructPair.png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
			nullptr, user_error_fn, user_warning_fn);
		if (infoStructPair.png_ptr == nullptr)
		{
			return std::unexpected<WriteError>({ErrorType::IOError,
				fmt::format("Failed to read {}: Failed to create png_struct for reading", path)
			});
		}

		infoStructPair.info_ptr = png_create_info_struct(infoStructPair.png_ptr);
		if (infoStructPair.info_ptr == nullptr)
		{
			return std::unexpected<WriteError>({ErrorType::IOError,
				fmt::format("Failed to read {}: Failed to create png_info for reading", path)
			});
		}

		png_init_io(infoStructPair.png_ptr, readFile.get());


		png_set_IHDR(
			infoStructPair.png_ptr,
			infoStructPair.info_ptr,
			image.width, image.height,
			8,
			PNG_COLOR_TYPE_GRAY,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_DEFAULT,
			PNG_FILTER_TYPE_DEFAULT
		);
		png_write_info(infoStructPair.png_ptr, infoStructPair.info_ptr);

		std::vector<const byte *> rows(image.height);
		for (std::size_t i = 0; i < image.height; ++i)
			rows[i] = &image[0, i];

		png_write_image(infoStructPair.png_ptr, const_cast<byte **>(rows.data()));

		return {};
	}
}
