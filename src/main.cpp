#include <chrono>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "helpers/texture.hpp"

const int pixel_size = 10;
const int randomness = 3;

enum class Field
{
	Unclicked,
	UnclickedAndChecked,
	Bomb,
	Clicked,
	One,
	Two,
	Three,
	Four,
	Five,
	Six,
	Seven,
	Eight,
};

int main(int argv, char** argc)
{
	cv::VideoCapture captured("/home/andraantariksa/Projects/minesweeperizer/assets/chika720p.mp4");

	if (!captured.isOpened())
	{
		std::cerr << "***Could not initialize capturing...***" << std::endl;
		return 1;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
		return 1;
	}

	int imgFlags = IMG_INIT_PNG;
	if(!(IMG_Init(imgFlags) & imgFlags))
	{
		std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
		return 1;
	}

	SDL_Window * window = nullptr;
	SDL_Renderer * renderer = nullptr;

	if (SDL_CreateWindowAndRenderer(1280, 720, 0, &window, &renderer) < 0)
	{
		std::cerr << "Error creating window or renderer: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}

	cv::Vec3b green_screen_color(0, 254, 10);

	SDL_Texture* texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_BGR24,
		SDL_TEXTUREACCESS_STREAMING,
		(int)captured.get(cv::CAP_PROP_FRAME_WIDTH),
		(int)captured.get(cv::CAP_PROP_FRAME_HEIGHT)
	);

	SDL_Texture* minesweeper_assets_button_clicked = IMG_LoadTexture(renderer, "../assets/block_clicked.png");
	SDL_Texture* minesweeper_assets_button_unclicked = IMG_LoadTexture(renderer, "../assets/block_unclicked.png");
	SDL_Texture* minesweeper_assets_button_smiley = IMG_LoadTexture(renderer, "../assets/smiley.png");
	SDL_Texture* minesweeper_assets_button_number_1 = IMG_LoadTexture(renderer, "../assets/1.png");
	SDL_Texture* minesweeper_assets_button_number_2 = IMG_LoadTexture(renderer, "../assets/2.png");
	SDL_Texture* minesweeper_assets_button_number_3 = IMG_LoadTexture(renderer, "../assets/3.png");
	SDL_Texture* minesweeper_assets_button_number_4 = IMG_LoadTexture(renderer, "../assets/4.png");
	SDL_Texture* minesweeper_assets_button_number_5 = IMG_LoadTexture(renderer, "../assets/5.png");
	SDL_Texture* minesweeper_assets_button_number_6 = IMG_LoadTexture(renderer, "../assets/6.png");
	SDL_Texture* minesweeper_assets_button_number_7 = IMG_LoadTexture(renderer, "../assets/7.png");
	SDL_Texture* minesweeper_assets_button_number_8 = IMG_LoadTexture(renderer, "../assets/8.png");

	SDL_Rect texture_rect;

	cv::Mat frame;

	double video_fps = captured.get(cv::CAP_PROP_FPS);
	double ticks_per_frame = 1000.0 / video_fps;

	std::cout << "Hot keys:" << std::endl;
	std::cout << "\tESC - quit the program" << std::endl;
	std::cout << "\tp - pause video" << std::endl;

	bool paused = false;
	bool quit = false;

	SDL_Event e;

	while (!quit)
	{
		auto time_before_main_loop = std::chrono::high_resolution_clock::now();

		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				quit = true;
				break;

			case SDL_KEYDOWN:
				switch (e.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					quit = true;
					break;

				case SDLK_p:
					paused = !paused;
					break;
				}
				break;
			}
		}

		if (!paused)
		{
			captured >> frame;

			if (frame.empty())
			{
				quit = true;
				continue;
			}
		}

		int minefield_row = frame.rows / (pixel_size * 2);
		int minefield_col = frame.cols / (pixel_size * 2);

		std::vector<std::vector<Field>> minefield(minefield_row, std::vector<Field>(minefield_col));

		SDL_RenderClear(renderer);

		// Fill Chika
//		Helpers::fillSDLTexture(texture, frame);

		// Render
//		SDL_RenderCopy(renderer, texture, nullptr, nullptr);

		// Distinguish green screen
		for (int rect_center_y = pixel_size, y_idx = 0; rect_center_y < frame.rows; rect_center_y += pixel_size * 2, y_idx++)
		{
			for (int rect_center_x = pixel_size, x_idx = 0; rect_center_x < frame.cols; rect_center_x += pixel_size * 2, x_idx++)
			{
				cv::Vec3b color = frame.at<cv::Vec3b>(rect_center_y, rect_center_x);

				if (cv::norm(color - green_screen_color) < 10.0)
				{
					minefield[y_idx][x_idx] = Field::Unclicked;
				}
				else
				{
					minefield[y_idx][x_idx] = Field::Clicked;
				}
			}
		}

		// Set bomb
		for (int y_idx = 0; y_idx < minefield.size(); y_idx++)
		{
			for (int x_idx = 0; x_idx < minefield[0].size(); x_idx++)
			{
				if (minefield[y_idx][x_idx] == Field::Clicked)
				{
					// Bruh moment may occur here
					// Top
					if (y_idx > 0)
					{
						if (x_idx > 0 && minefield[y_idx - 1][x_idx - 1] == Field::Unclicked)
						{
							minefield[y_idx - 1][x_idx - 1] = (rand() % randomness == 0) ? Field::Bomb
																						 : Field::UnclickedAndChecked;
						}

						if (minefield[y_idx - 1][x_idx] == Field::Unclicked)
						{
							minefield[y_idx - 1][x_idx] = (rand() % randomness == 0) ? Field::Bomb
																					 : Field::UnclickedAndChecked;
						}

						if (x_idx < minefield[0].size() - 1 && minefield[y_idx - 1][x_idx + 1] == Field::Unclicked)
						{
							minefield[y_idx - 1][x_idx + 1] = (rand() % randomness == 0) ? Field::Bomb
																						 : Field::UnclickedAndChecked;
						}
					}

					// Middle
					if (x_idx > 0 && minefield[y_idx][x_idx - 1] == Field::Unclicked)
					{
						minefield[y_idx][x_idx - 1] = (rand() % randomness == 0)? Field::Bomb : Field::UnclickedAndChecked;
					}

					if (minefield[y_idx][x_idx] == Field::Unclicked)
					{
						minefield[y_idx][x_idx] = (rand() % randomness == 0)? Field::Bomb : Field::UnclickedAndChecked;
					}

					if (x_idx < minefield[0].size() - 1 && minefield[y_idx][x_idx + 1] == Field::Unclicked)
					{
						minefield[y_idx][x_idx + 1] = (rand() % randomness == 0)? Field::Bomb : Field::UnclickedAndChecked;
					}

					// Bottom
					if (y_idx < minefield.size() - 1)
					{
						if (x_idx > 0 && minefield[y_idx + 1][x_idx - 1] == Field::Unclicked)
						{
							minefield[y_idx + 1][x_idx - 1] = (rand() % randomness == 0) ? Field::Bomb
																						 : Field::UnclickedAndChecked;
						}

						if (minefield[y_idx + 1][x_idx] == Field::Unclicked)
						{
							minefield[y_idx + 1][x_idx] = (rand() % randomness == 0) ? Field::Bomb
																					 : Field::UnclickedAndChecked;
						}

						if (x_idx < minefield[0].size() - 1 && minefield[y_idx + 1][x_idx + 1] == Field::Unclicked)
						{
							minefield[y_idx + 1][x_idx + 1] = (rand() % randomness == 0) ? Field::Bomb
																						 : Field::UnclickedAndChecked;
						}
					}
				}
			}
		}

		// Set number
		for (int y_idx = 0; y_idx < minefield.size(); y_idx++)
		{
			for (int x_idx = 0; x_idx < minefield[0].size(); x_idx++)
			{
				int total_bomb = 0;

				if (minefield[y_idx][x_idx] == Field::Clicked)
				{
					// Bruh moment may occur here
					// Top
					if (y_idx > 0)
					{
						if (x_idx > 0 && minefield[y_idx - 1][x_idx - 1] == Field::Bomb)
						{
							if (minefield[y_idx - 1][x_idx - 1] == Field::Bomb)
							{
								total_bomb += 1;
							}
						}

						if (minefield[y_idx - 1][x_idx] == Field::Bomb)
						{
							if(minefield[y_idx - 1][x_idx] == Field::Bomb)
							{
								total_bomb += 1;
							}
						}

						if (x_idx < minefield[0].size() - 1 && minefield[y_idx - 1][x_idx + 1] == Field::Bomb)
						{
							if (minefield[y_idx - 1][x_idx + 1] == Field::Bomb)
							{
								total_bomb += 1;
							}
						}
					}

					// Middle
					if (x_idx > 0 && minefield[y_idx][x_idx - 1] == Field::Bomb)
					{
						if(minefield[y_idx][x_idx - 1] == Field::Bomb)
						{
							total_bomb += 1;
						}
					}

					if (minefield[y_idx][x_idx] == Field::Bomb)
					{
						if(minefield[y_idx][x_idx] == Field::Bomb)
						{
							total_bomb += 1;
						}
					}

					if (x_idx < minefield[0].size() - 1 && minefield[y_idx][x_idx + 1] == Field::Bomb)
					{
						if(minefield[y_idx][x_idx + 1] == Field::Bomb)
						{
							total_bomb += 1;
						}
					}

					// Bottom
					if (y_idx < minefield.size() - 1)
					{
						if (x_idx > 0 && minefield[y_idx + 1][x_idx - 1] == Field::Bomb)
						{
							if(minefield[y_idx + 1][x_idx - 1] == Field::Bomb)
							{
								total_bomb += 1;
							}
						}

						if (minefield[y_idx + 1][x_idx] == Field::Bomb)
						{
							if(minefield[y_idx + 1][x_idx] == Field::Bomb)
							{
								total_bomb += 1;
							}
						}

						if (x_idx < minefield[0].size() - 1 && minefield[y_idx + 1][x_idx + 1] == Field::Bomb)
						{
							if(minefield[y_idx + 1][x_idx + 1] == Field::Bomb)
							{
								total_bomb += 1;
							}
						}
					}

					if (total_bomb == 0)
					{
						minefield[y_idx][x_idx] = Field::Clicked;
					}
					else if (total_bomb == 1)
					{
						minefield[y_idx][x_idx] = Field::One;
					}
					else if (total_bomb == 2)
					{
						minefield[y_idx][x_idx] = Field::Two;
					}
					else if (total_bomb == 3)
					{
						minefield[y_idx][x_idx] = Field::Three;
					}
					else if (total_bomb == 4)
					{
						minefield[y_idx][x_idx] = Field::Four;
					}
					else if (total_bomb == 5)
					{
						minefield[y_idx][x_idx] = Field::Five;
					}
					else if (total_bomb == 6)
					{
						minefield[y_idx][x_idx] = Field::Six;
					}
					else if (total_bomb == 7)
					{
						minefield[y_idx][x_idx] = Field::Seven;
					}
					else if (total_bomb == 8)
					{
						minefield[y_idx][x_idx] = Field::Eight;
					}
				}
			}
		}

		for (int y_idx = 0; y_idx < minefield.size(); y_idx++)
		{
			for (int x_idx = 0; x_idx < minefield[0].size(); x_idx++)
			{
				texture_rect.w = pixel_size * 2;
				texture_rect.h = pixel_size * 2;
				texture_rect.x = (2 * pixel_size * x_idx) - pixel_size;
				texture_rect.y = (2 * pixel_size * y_idx) - pixel_size;


				if (minefield[y_idx][x_idx] == Field::Clicked)
				{
					SDL_RenderCopy(renderer, minesweeper_assets_button_clicked, nullptr, &texture_rect);
				}
				else if (minefield[y_idx][x_idx] == Field::One)
				{
					SDL_RenderCopy(renderer, minesweeper_assets_button_number_1, nullptr, &texture_rect);
				}
				else if (minefield[y_idx][x_idx] == Field::Two)
				{
					SDL_RenderCopy(renderer, minesweeper_assets_button_number_2, nullptr, &texture_rect);
				}
				else if (minefield[y_idx][x_idx] == Field::Three)
				{
					SDL_RenderCopy(renderer, minesweeper_assets_button_number_3, nullptr, &texture_rect);
				}
				else if (minefield[y_idx][x_idx] == Field::Four)
				{
					SDL_RenderCopy(renderer, minesweeper_assets_button_number_4, nullptr, &texture_rect);
				}
				else if (minefield[y_idx][x_idx] == Field::Five)
				{
					SDL_RenderCopy(renderer, minesweeper_assets_button_number_5, nullptr, &texture_rect);
				}
				else if (minefield[y_idx][x_idx] == Field::Six)
				{
					SDL_RenderCopy(renderer, minesweeper_assets_button_number_6, nullptr, &texture_rect);
				}
				else if (minefield[y_idx][x_idx] == Field::Seven)
				{
					SDL_RenderCopy(renderer, minesweeper_assets_button_number_7, nullptr, &texture_rect);
				}
				else if (minefield[y_idx][x_idx] == Field::Eight)
				{
					SDL_RenderCopy(renderer, minesweeper_assets_button_number_8, nullptr, &texture_rect);
				}
				else
				{
					SDL_RenderCopy(renderer, minesweeper_assets_button_unclicked, nullptr, &texture_rect);
				}
			}
		}

		SDL_RenderPresent(renderer);

		auto time_after_main_loop = std::chrono::high_resolution_clock::now();

		double delta_time_ms = std::chrono::duration<double, std::milli>(time_after_main_loop - time_before_main_loop).count();

		if (delta_time_ms < ticks_per_frame)
		{
			SDL_Delay(ticks_per_frame - delta_time_ms);
		}
	}

	captured.release();

	SDL_DestroyTexture(texture);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
