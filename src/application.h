#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <memory>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

class Application
{
private:
	int pixel_size;
	int randomness;
	double color_tolerance;

	cv::VideoCapture video_captured;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	cv::Vec3b green_screen_color;
	SDL_Rect texture_rectangle;

	struct
	{
		SDL_Texture* minesweeper_field_one;
		SDL_Texture* minesweeper_field_two;
		SDL_Texture* minesweeper_field_three;
		SDL_Texture* minesweeper_field_four;
		SDL_Texture* minesweeper_field_five;
		SDL_Texture* minesweeper_field_six;
		SDL_Texture* minesweeper_field_seven;
		SDL_Texture* minesweeper_field_eight;
		SDL_Texture* minesweeper_field_clicked;
		SDL_Texture* minesweeper_field_unclicked;
		SDL_Texture* minesweeper_field_smiley;
	}
	assets;

	void loadAssets();
	void freeAssets();

public:
	Application(const char* filename, const cv::Vec3b& green_screen_color, const int pixel_size, const int randomness, const double color_tolerance);
	~Application();

	void dispatch();
};

#endif //_APPLICATION_H_
