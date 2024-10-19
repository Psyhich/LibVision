#include <cxxopts.hpp>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include "filters.h"
#include "image_io.h"
#include "math.h"

std::vector<const char *> create_args_from_unmatched(std::vector<std::string> &unmatched)
{
	std::vector<const char *> args;
	args.reserve(unmatched.size() + 1);
	args.push_back("./prog");
	for (auto &str : unmatched)
		args.push_back(str.data());

	return args;
}

template<typename T, typename FieldType, FieldType T::*FieldPtr>
struct StructLessCmp
{
	bool operator()(const T &l_val, const T &r_val) const
	{
		return l_val.*FieldPtr < r_val.*FieldPtr;
	}
};

template<typename T>
struct member_type_helper
{};

template <class C, class T>
struct member_type_helper<T C::*> { using type = T; };

template<typename T, auto FieldPtr>
using less_cmp = StructLessCmp<T, typename member_type_helper<typename std::remove_cvref_t<decltype(FieldPtr)>>::type, FieldPtr>;

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
	bool actionHappend = true;
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
	}
	else if (filter == "top-hat")
	{
		cxxopts::Options options{"Top hat filter"};
		options.add_options()
			("I,inner-radius", "Inner cirlce radius", cxxopts::value<int>()->default_value("3"))
			("O,outter-radius", "Outter cirlce radius", cxxopts::value<int>()->default_value("5"))
			("T,threshold", "Threshold", cxxopts::value<std::size_t>()->default_value("10"))
			("D,dark", "Fill with dark", cxxopts::value<int>()->default_value("1"));
		const auto args{create_args_from_unmatched(unmatched)};
		const auto result{options.parse(args.size(), args.data())};

		const int inner_radius{result["inner-radius"].as<int>()};
		const int outter_radius{result["outter-radius"].as<int>()};
		const std::size_t threshold{result["threshold"].as<std::size_t>()};
		const int dark{result["dark"].as<int>()};

		vl::filters::top_hat(image, inner_radius, outter_radius, threshold, dark);
	}
	else if (filter == "rolling-ball")
	{
		cxxopts::Options options{"Rolling ball filter"};
		options.add_options()
			("I,inner-radius", "Inner cirlce radius", cxxopts::value<int>()->default_value("3"))
			("O,outter-radius", "Outter cirlce radius", cxxopts::value<int>()->default_value("5"))
			("T,threshold", "Threshold", cxxopts::value<std::size_t>()->default_value("10"))
			("D,dark", "Fill with dark", cxxopts::value<int>()->default_value("1"));
		const auto args{create_args_from_unmatched(unmatched)};
		const auto result{options.parse(args.size(), args.data())};

		const int inner_radius{result["inner-radius"].as<int>()};
		const int outter_radius{result["outter-radius"].as<int>()};
		const std::size_t threshold{result["threshold"].as<std::size_t>()};
		const int dark{result["dark"].as<int>()};

		vl::filters::rolling_ball(image, inner_radius, outter_radius, threshold, dark);
	}
	else if (filter == "none")
	{
		actionHappend = false;
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
