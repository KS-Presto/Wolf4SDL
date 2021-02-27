#ifndef __SDL_WRAP
#define __SDL_WRAP

#include <SDL.h>
#if SDL_MAJOR_VERSION == 2
void Present(SDL_Surface *screen);
#endif
#endif
