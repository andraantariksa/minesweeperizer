#include <cxxopts.hpp>
#include <opencv2/core/core.hpp>

#include "application.h"

int main(int argc, char** argv)
{
	cxxopts::Options options("Minesweeperizer", "Convert a video to a minesweeper!");

	options.add_options()
		("file", "Video file", cxxopts::value<std::string>())
		("pixel-size", "Pixel size", cxxopts::value<int>()->default_value("10"))
		("tolerance", "Color tolerance (0.0 - 255.0)", cxxopts::value<double>()->default_value("10.0"))
		("randomness", "Randomness", cxxopts::value<int>()->default_value("5"))
		("greenscreen", "Greenscreen Color", cxxopts::value<std::string>()->default_value("0,254,10"))
		("h,help", "Help")
	;

	auto result = options.parse(argc, argv);

	if (result.count("help"))
	{
		std::cout << options.help() << std::endl;
		exit(0);
	}

	int r, g, b;
	std::stringstream ss(result["greenscreen"].as<std::string>());
	ss >> r;
	ss.ignore(1, ',');
	ss >> g;
	ss.ignore(1, ',');
	ss >> b;

	Application app(result["file"].as<std::string>().c_str(), cv::Vec3b(r, g, b), result["pixel-size"].as<int>(), result["randomness"].as<int>(), result["tolerance"].as<double>());
	app.dispatch();

	return 0;
}
