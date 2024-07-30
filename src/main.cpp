#include <cxxopts.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "filters.h"
#include "image_io.h"

std::vector<char *> create_args_from_unmatched(std::vector<std::string> &unmatched)
{
	std::vector<char *> args;
	args.reserve(unmatched.size());
	for (auto &str : unmatched)
	{
		args.push_back(str.data());
	}

	return args;
}

int main(int argc, char **argv)
{
	cxxopts::Options options{"Filters", "This is example program of using filters lib"};

	options.add_options()
		("i,input", "Input file", cxxopts::value<std::string>())
		("f,filter", "Filter to use", cxxopts::value<std::string>())
		("o,output", "Output file", cxxopts::value<std::string>());
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

	const auto filter{result["filter"].as<std::string>()};
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

		vl::filters::gaussian_filter(image, stdDev, size);
	}

	const auto writeResult{vl::ImageIO::write_png(image, result["output"].as<std::string>())};
	if (!writeResult)
	{
		fmt::println("Got error while writting: {}", writeResult.error().description);
		return -1;
	}
	return 0;
}
