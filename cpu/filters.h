//#include "sdl_aux.h"
//#ifndef __FILTERS_H__
//#define __FILTERS_H__
SDL_Surface* filter_depth( SDL_Surface* depth_frame,
                           SDL_Surface* depth_frame_filtered,
                           int x, int y,
                           unsigned int width, unsigned int height);

#define GAUSSIAN_KERNEL_SIZE 9
void calc_gaussian_kernel(double sigmax, double sigmay);
//#endif
