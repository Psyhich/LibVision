#include <cxxopts.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "filters.h"
#include "image_io.h"
#include "math.h"
#include "segmentation.h"

std::vector<const char *> create_args_from_unmatched(std::vector<std::string> &unmatched)
{
	std::vector<const char *> args;
	args.reserve(unmatched.size() + 1);
	args.push_back("./prog");
	for (auto &str : unmatched)
		args.push_back(str.data());

	return args;
}

int main(int argc, char **argv)
{
	cxxopts::Options options{"Filters", "This is example program of using filters lib"};

	options.add_options()
		("i,input", "Input file", cxxopts::value<std::string>())
		("c,calc", "Calculation to use", cxxopts::value<std::string>()->default_value("none"))
		("f,filter", "Filter to use", cxxopts::value<std::string>()->default_value("none"))
		("o,output", "Output file", cxxopts::value<std::string>()->default_value("output.png"));
	options.allow_unrecognised_options();
	const auto result{options.parse(argc, argv)};
	auto unmatched{result.unmatched()};

	auto readImage{vl::ImageIO::read_png(result["input"].as<std::string>())};
	if (!readImage.has_value())
	{
		fmt::println("Failed to read:\n{}", readImage.error().description);
		return -1;
	}
	auto &image{readImage.value()};

	const auto calc{result["calc"].as<std::string>()};
	if (calc == "entropy")
	{
		fmt::println("{} entropy level: {}", result["input"].as<std::string>(), vl::math::entropy(image));
	}
	else if (calc == "snr")
	{
		fmt::println("{} signal to noise ratio: {}", result["input"].as<std::string>(), vl::math::signal_to_noise_ratio(image));
	}
	const auto filter{result["filter"].as<std::string>()};
	bool actionHappend = false;
	if (filter == "gauss")
	{
		cxxopts::Options options{"Gauss filter"};
		options.add_options()
			("d,std-dev", "Standard deviation", cxxopts::value<double>()->default_value("1"))
			("s,size", "Kernel size", cxxopts::value<std::size_t>()->default_value("3"));
		const auto args{create_args_from_unmatched(unmatched)};
		const auto result{options.parse(args.size(), args.data())};

		const auto stdDev{result["std-dev"].as<double>()};
		const auto size{result["size"].as<std::size_t>()};

		vl::filters::gaussian(image, stdDev, size);
		actionHappend = true;
	}
	else if (filter == "median")
	{
		cxxopts::Options options{"Median filter"};
		options.add_options()
			("s,size", "Kernel size", cxxopts::value<std::size_t>()->default_value("3"))
			("S,shape", "Filter shape", cxxopts::value<std::string>()->default_value("rectangle"));
		const auto args{create_args_from_unmatched(unmatched)};
		const auto result{options.parse(args.size(), args.data())};


		const auto size{result["size"].as<std::size_t>()};
		const auto shapeString{result["shape"].as<std::string>()};
		const auto shape{vl::filters::to_shape(shapeString)};
		if (!shape)
		{
			fmt::println("Invalid shape name: {}", shapeString);
			return -1;
		}

		vl::filters::median(image, size, *shape);
		actionHappend = true;
	}
	else if (filter == "truncated-median")
	{
		cxxopts::Options options{"Truncated median filter"};
		options.add_options()
			("s,size", "Kernel size", cxxopts::value<std::size_t>()->default_value("3"))
			("c,std-dev-count", "Count of standard eviation to accept", cxxopts::value<std::size_t>()->default_value("2"))
			("S,shape", "Filter shape", cxxopts::value<std::string>()->default_value("rectangle"));
		const auto args{create_args_from_unmatched(unmatched)};
		const auto result{options.parse(args.size(), args.data())};


		const auto size{result["size"].as<std::size_t>()};
		const auto stdDevCount{result["std-dev-count"].as<std::size_t>()};
		const auto shapeString{result["shape"].as<std::string>()};
		const auto shape{vl::filters::to_shape(shapeString)};
		if (!shape)
		{
			fmt::println("Invalid shape name: {}", shapeString);
			return -1;
		}

		vl::filters::truncated_median(image, size, stdDevCount, *shape);
		actionHappend = true;
	}
	else if (filter == "hybrid-median")
	{
		cxxopts::Options options{"Hybird median filter"};
		options.add_options()
			("s,size", "Kernel size", cxxopts::value<std::size_t>()->default_value("3"))
			("c,std-dev-count", "Count of standard eviation to accept", cxxopts::value<std::size_t>()->default_value("2"))
			("S,shape", "Filter shape", cxxopts::value<std::string>()->default_value("rectangle"));
		const auto args{create_args_from_unmatched(unmatched)};
		const auto result{options.parse(args.size(), args.data())};

		const auto size{result["size"].as<std::size_t>()};
		vl::filters::hybrid_median(image, size);
		actionHappend = true;
	}
	else if (filter == "erosion")
	{
		cxxopts::Options options{"Erosion filter"};
		options.add_options()
			("s,size", "Kernel size", cxxopts::value<std::size_t>()->default_value("3"))
			("S,shape", "Filter shape", cxxopts::value<std::string>()->default_value("rectangle"));
		const auto args{create_args_from_unmatched(unmatched)};
		const auto result{options.parse(args.size(), args.data())};

		const auto size{result["size"].as<std::size_t>()};
		const auto shapeString{result["shape"].as<std::string>()};
		const vl::filters::Shape shape{*vl::filters::to_shape(shapeString)};
		vl::filters::erosion(image, shape, size);
		actionHappend = true;
	}
	else if (filter == "dilation")
	{
		cxxopts::Options options{"Dilation filter"};
		options.add_options()
			("s,size", "Kernel size", cxxopts::value<std::size_t>()->default_value("3"))
			("S,shape", "Filter shape", cxxopts::value<std::string>()->default_value("rectangle"));
		const auto args{create_args_from_unmatched(unmatched)};
		const auto result{options.parse(args.size(), args.data())};

		const auto size{result["size"].as<std::size_t>()};
		const auto shapeString{result["shape"].as<std::string>()};
		const vl::filters::Shape shape{*vl::filters::to_shape(shapeString)};
		vl::filters::dilation(image, shape, size);
		actionHappend = true;
	}
	else if (filter == "top-hat")
	{
		cxxopts::Options options{"Top hat filter"};
		options.add_options()
			("s,size", "Kernel size", cxxopts::value<std::size_t>()->default_value("3"))
			("S,shape", "Filter shape", cxxopts::value<std::string>()->default_value("rectangle"));
		const auto args{create_args_from_unmatched(unmatched)};
		const auto result{options.parse(args.size(), args.data())};

		const auto size{result["size"].as<std::size_t>()};
		const auto shapeString{result["shape"].as<std::string>()};
		const vl::filters::Shape shape{*vl::filters::to_shape(shapeString)};
		vl::Image copy{image};
		vl::filters::erosion(copy, shape, size);
		vl::filters::dilation(copy, shape, size);

		for (std::size_t y = 0; y < copy.height; ++y)
			for (std::size_t x = 0; x < copy.width; ++x)
				image[x, y] = std::max((int)image[x, y] - copy[x, y], 0);
		actionHappend = true;
	}
	else if (filter == "segment")
	{
		const auto params{vl::compute_sector_parameters(image)};
		const std::size_t sector_height{image.height / params.rows()};
		const std::size_t sector_width{image.width / params.cols()};

		for (std::size_t sector_row = 0; sector_row < params.rows(); ++sector_row)
		{
			const std::size_t sector_y_start{sector_row * sector_height};
			for (std::size_t sector_col = 0; sector_col < params.cols(); ++sector_col)
			{
				const std::size_t sector_x_start{sector_col * sector_width};
				for (std::size_t row = 0; row < sector_height; ++row)
					for (std::size_t col = 0; col < sector_width; ++col)
						image[sector_x_start + col, sector_y_start + row] = std::min(params[sector_row, sector_col].angular_second_moment * 255000., 255.);
			}
		}

		actionHappend = true;
	}
	else if (filter == "none")
	{
	}
	else
	{
		fmt::println("Unrecognized option filter: {}", filter);
		return -1;
	}

	if (actionHappend)
	{
		const auto writeResult{vl::ImageIO::write_png(image, result["output"].as<std::string>())};
		if (!writeResult)
		{
			fmt::println("Got error while writting: {}", writeResult.error().description);
			return -1;
		}
	}

	return 0;
}
