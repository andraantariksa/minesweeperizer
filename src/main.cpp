#include <cxxopts.hpp>
#include <opencv2/core/core.hpp>

#include "application.h"

int main(int argc, char** argv)
{
	cxxopts::Options options("Minesweeperizer", "Convert a video to a minesweeper!");

	options.add_options()
		("file", "Video file", cxxopts::value<std::string>())
		("pixel-size", "Pixel size", cxxopts::value<int>()->default_value("10"))
		("tolerance", "Color tolerance (0.0 - 255.0)", cxxopts::value<double>()->default_value("50.0"))
		("randomness", "Randomness", cxxopts::value<int>()->default_value("5"))
		("greenscreen", "Greenscreen Color", cxxopts::value<std::vector<short>>()->default_value("0,254,10"))
		("h,help", "Help")
	;

	std::unique_ptr<cxxopts::ParseResult> result;
	try
	{
		result = std::make_unique<cxxopts::ParseResult>(options.parse(argc, argv));
	}
	catch(std::exception err)
	{
		exit(1);
	}

	if (result->count("help"))
	{
		std::cout << options.help() << std::endl;
		exit(0);
	}

	std::vector<short> color = result->operator[]("greenscreen").as<std::vector<short>>();

	Application app(
		result->operator[]("file").as<std::string>().c_str(),
		cv::Vec3b(color[0], color[1], color[2]),
		result->operator[]("pixel-size").as<int>(),
		result->operator[]("randomness").as<int>(),
		result->operator[]("tolerance").as<double>()
		);
	app.dispatch();

	return 0;
}
