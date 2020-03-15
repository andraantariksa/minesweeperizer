#ifndef _HELPERS_TEXTURE
#define _HELPERS_TEXTURE

#include <opencv2/core.hpp>
#include <SDL2/SDL.h>

namespace Helpers
{

	void fillSDLTexture(SDL_Texture * texture, cv::Mat const &mat);
//	std::unique_ptr<SDL_Surface> loadImage(const char * file_path);
}

#endif // _HELPERS_TEXTURE
