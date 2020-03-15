#include <opencv2/core.hpp>
#include <opencv2/core/types_c.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

namespace Helpers
{

	void fillSDLTexture(SDL_Texture * texture, cv::Mat const &mat)
	{
		IplImage ipl_image = cvIplImage(mat);

		unsigned char * texture_data = nullptr;
		int texture_pitch = 0;

		SDL_LockTexture(texture, nullptr, (void **)&texture_data, &texture_pitch);
		memcpy(texture_data, (void *)ipl_image.imageData, ipl_image.width * ipl_image.height * ipl_image.nChannels);
		SDL_UnlockTexture(texture);
	}

//	std::unique_ptr<SDL_Surface> loadImage(const char * file_path)
//	{
//		std::unique_ptr<SDL_Surface> image_surface(IMG_LoadTexture(, file_path));
//		return std::move(image_surface);
//	}

}