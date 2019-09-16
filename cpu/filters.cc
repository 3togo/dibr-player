#include <math.h>
#include <algorithm> //min,max
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_image.h"
#include "sdl_aux.h"

#define GAUSSIAN_KERNEL_SIZE 9
double gaussian_kernel[GAUSSIAN_KERNEL_SIZE][GAUSSIAN_KERNEL_SIZE];

/***************** Depth filtering related functions **************************/
double gaussian (double x, double mu, double sigma)
{
  return exp ( -(((x-mu)/(sigma))*((x-mu)/(sigma)))/2.0 );
}

double oriented_gaussian (int x, int y,
                          double sigma1, double sigma2)
{
  return exp ( - ((x)*(x))/(2.0*sigma1) - ((y)*(y))/(2.0*sigma2));
}

void calc_gaussian_kernel(double sigmax, double sigmay)
{
  double sum = 0;
  for (int x = 0; x < GAUSSIAN_KERNEL_SIZE; x++)
  {
    for (int y = 0; y < GAUSSIAN_KERNEL_SIZE; y++)
    {
      int x1 = x - GAUSSIAN_KERNEL_SIZE/2;
      int y1 = y - GAUSSIAN_KERNEL_SIZE/2;
      gaussian_kernel[x][y] = oriented_gaussian(x1, y1, sigmax, sigmay);
      sum += gaussian_kernel[x][y];
    }
  }

  for (int x = 0; x < GAUSSIAN_KERNEL_SIZE; x++)
    for (int y = 0; y < GAUSSIAN_KERNEL_SIZE; y++)
      gaussian_kernel[x][y] /= sum;
}

SDL_Surface* filter_depth( SDL_Surface* depth_frame,
                           SDL_Surface* depth_frame_filtered,
                           int x, int y,
                           unsigned int width, unsigned int height)
{
  int cols = width;
  int rows = height;

  int neighborX, neighborY;
  Uint8 r, g, b;
  Uint8 r_new, g_new, b_new;
  float r_sum, g_sum, b_sum;

  for (int y = GAUSSIAN_KERNEL_SIZE-1; y < rows - GAUSSIAN_KERNEL_SIZE-1; y++)
  {
    for (int x = GAUSSIAN_KERNEL_SIZE-1; x < cols-GAUSSIAN_KERNEL_SIZE-1; x++)
    {
      r_sum = 0;
      g_sum = 0;
      b_sum = 0;

      for (int filterX = 0; filterX < GAUSSIAN_KERNEL_SIZE; filterX++)
      {
        for (int filterY = 0; filterY < GAUSSIAN_KERNEL_SIZE; filterY++)
        {
          // get original depth around point being calculated
          neighborX = x - 1 + filterX;
          neighborY = y - 1 + filterY;

          // get neighbor pixels
          Uint32 pixel = sdl_get_pixel(depth_frame, neighborX, neighborY);
          SDL_GetRGB (pixel, depth_frame->format, &r, &g, &b);

          r_sum += r * gaussian_kernel[filterX][filterY];
          g_sum += g * gaussian_kernel[filterX][filterY];
          b_sum += b * gaussian_kernel[filterX][filterY];
          // OK to apply Guassian filter to RGB values instead of Y value of YUV?
          // testing Ybefore and Yafter suggests its ok
        }

        r_new = std::min(std::max(int( r_sum ), 0), 255); // not using bias or factor
        g_new = std::min(std::max(int( g_sum ), 0), 255);
        b_new = std::min(std::max(int( b_sum ), 0), 255);

        // store result in original position in depth_frame_filtered
        Uint32 pixel_new = SDL_MapRGB( depth_frame_filtered->format,
                                       r_new, g_new, b_new );
        sdl_put_pixel (depth_frame_filtered, x, y, pixel_new );
      }
    }
  }
  return depth_frame_filtered;
}

