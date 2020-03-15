#include "application.h"

#include <iostream>
#include <chrono>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "field.h"
#include "helpers/texture.h"

Application::Application(const char* filename, const cv::Vec3b& green_screen_color, const int pixel_size, const int randomness, const double color_tolerance) :
	video_captured(filename),
	green_screen_color(green_screen_color),
	pixel_size(pixel_size),
	randomness(randomness),
	color_tolerance(color_tolerance)
{
	if (!video_captured.isOpened())
	{
		throw std::runtime_error("Could not initialize video capturing");
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::string error_msg("Failed to initialize SDL: ");
		error_msg += SDL_GetError();
		throw std::runtime_error(error_msg);
	}

	int imgFlags = IMG_INIT_PNG;
	if(!(IMG_Init(imgFlags) & imgFlags))
	{
		std::string error_msg("SDL_image could not initialize! SDL_image Error: ");
		error_msg += IMG_GetError();
		throw std::runtime_error(error_msg);
	}

	if (SDL_CreateWindowAndRenderer(
		(int)video_captured.get(cv::CAP_PROP_FRAME_WIDTH),
		(int)video_captured.get(cv::CAP_PROP_FRAME_HEIGHT),
		0,
		&window,
		&renderer
		) < 0)
	{
		std::string error_msg("Error creating window or renderer: ");
		error_msg += SDL_GetError();
		throw std::runtime_error(error_msg);
	}

	texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_BGR24,
		SDL_TEXTUREACCESS_STREAMING,
		(int)video_captured.get(cv::CAP_PROP_FRAME_WIDTH),
		(int)video_captured.get(cv::CAP_PROP_FRAME_HEIGHT)
		);

	loadAssets();
}

Application::~Application()
{
	freeAssets();

	video_captured.release();

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}

void Application::dispatch()
{
	std::cout << "Hot keys:" << std::endl;
	std::cout << "\tESC - quit the program" << std::endl;
	std::cout << "\tp - pause video" << std::endl;

	double frame_per_second = video_captured.get(cv::CAP_PROP_FPS);
	double milisec_per_frame = 1000.0 / frame_per_second;
	std::chrono::time_point<std::chrono::system_clock> time_before_process_frame, time_after_process_frame;
	double delta_time_ms;

	cv::Mat frame;

	bool paused = false;
	bool quit = false;

	SDL_Event event;

	while (!quit)
	{
		time_before_process_frame = std::chrono::high_resolution_clock::now();

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				quit = true;
				break;

			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
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

		// If pause then dont get a new frame
		if (!paused)
		{
			video_captured >> frame;

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

				if (cv::norm(color - green_screen_color) < color_tolerance)
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
				texture_rectangle.w = pixel_size * 2;
				texture_rectangle.h = pixel_size * 2;
				texture_rectangle.x = (2 * pixel_size * x_idx);
				texture_rectangle.y = (2 * pixel_size * y_idx);


				if (minefield[y_idx][x_idx] == Field::Clicked)
				{
					SDL_RenderCopy(renderer, assets.minesweeper_field_clicked, nullptr, &texture_rectangle);
				}
				else if (minefield[y_idx][x_idx] == Field::One)
				{
					SDL_RenderCopy(renderer, assets.minesweeper_field_one, nullptr, &texture_rectangle);
				}
				else if (minefield[y_idx][x_idx] == Field::Two)
				{
					SDL_RenderCopy(renderer, assets.minesweeper_field_two, nullptr, &texture_rectangle);
				}
				else if (minefield[y_idx][x_idx] == Field::Three)
				{
					SDL_RenderCopy(renderer, assets.minesweeper_field_three, nullptr, &texture_rectangle);
				}
				else if (minefield[y_idx][x_idx] == Field::Four)
				{
					SDL_RenderCopy(renderer, assets.minesweeper_field_four, nullptr, &texture_rectangle);
				}
				else if (minefield[y_idx][x_idx] == Field::Five)
				{
					SDL_RenderCopy(renderer, assets.minesweeper_field_five, nullptr, &texture_rectangle);
				}
				else if (minefield[y_idx][x_idx] == Field::Six)
				{
					SDL_RenderCopy(renderer, assets.minesweeper_field_six, nullptr, &texture_rectangle);
				}
				else if (minefield[y_idx][x_idx] == Field::Seven)
				{
					SDL_RenderCopy(renderer, assets.minesweeper_field_seven, nullptr, &texture_rectangle);
				}
				else if (minefield[y_idx][x_idx] == Field::Eight)
				{
					SDL_RenderCopy(renderer, assets.minesweeper_field_eight, nullptr, &texture_rectangle);
				}
				else
				{
					SDL_RenderCopy(renderer, assets.minesweeper_field_unclicked, nullptr, &texture_rectangle);
				}
			}
		}

		// Render
		SDL_RenderPresent(renderer);

		// Delta time
		time_after_process_frame = std::chrono::high_resolution_clock::now();
		delta_time_ms = std::chrono::duration<double, std::milli>(time_after_process_frame - time_before_process_frame).count();
		if (delta_time_ms < milisec_per_frame)
		{
			SDL_Delay(milisec_per_frame - delta_time_ms);
		}
	}
}

void Application::loadAssets()
{
	assets.minesweeper_field_one = IMG_LoadTexture(renderer, "../assets/1.png");
	assets.minesweeper_field_two = IMG_LoadTexture(renderer, "../assets/2.png");
	assets.minesweeper_field_three = IMG_LoadTexture(renderer, "../assets/3.png");
	assets.minesweeper_field_four = IMG_LoadTexture(renderer, "../assets/4.png");
	assets.minesweeper_field_five = IMG_LoadTexture(renderer, "../assets/5.png");
	assets.minesweeper_field_six = IMG_LoadTexture(renderer, "../assets/6.png");
	assets.minesweeper_field_seven = IMG_LoadTexture(renderer, "../assets/7.png");
	assets.minesweeper_field_eight = IMG_LoadTexture(renderer, "../assets/8.png");
	assets.minesweeper_field_clicked = IMG_LoadTexture(renderer, "../assets/block_clicked.png");
	assets.minesweeper_field_unclicked = IMG_LoadTexture(renderer, "../assets/block_unclicked.png");
	assets.minesweeper_field_smiley = IMG_LoadTexture(renderer, "../assets/smiley.png");
}

void Application::freeAssets()
{
	SDL_DestroyTexture(assets.minesweeper_field_one);
	SDL_DestroyTexture(assets.minesweeper_field_two);
	SDL_DestroyTexture(assets.minesweeper_field_three);
	SDL_DestroyTexture(assets.minesweeper_field_four);
	SDL_DestroyTexture(assets.minesweeper_field_five);
	SDL_DestroyTexture(assets.minesweeper_field_six);
	SDL_DestroyTexture(assets.minesweeper_field_seven);
	SDL_DestroyTexture(assets.minesweeper_field_eight);
	SDL_DestroyTexture(assets.minesweeper_field_eight);
	SDL_DestroyTexture(assets.minesweeper_field_clicked);
	SDL_DestroyTexture(assets.minesweeper_field_unclicked);
	SDL_DestroyTexture(assets.minesweeper_field_smiley);
}
