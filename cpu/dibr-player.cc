/* This file is part of dibr-player.
 *
 * dibr-player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dibr-player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 */
/*************** DIBR Overview *************************************************
 * Depth-image-based rendering is the process of generating virtual views from
 * a set of original view and associated depth frame. In special, this project
 * is intended to generate a stereo-pair from a reference texture. The following
 * 'image' shows schematically the process.
 *   ___________ ___________
 *  |           |           |
 *  |  Texture  |   Depth   |------------ INPUT
 *  |___________|___________|
 *        |           |
 *        |      _____|_____
 *        |     |           |
 *        |     |   Depth   |
 *        |     | Filtering |
 *        |     |___________|
 *        |___________|
 *              |
 *        _____\|/______
 *       |              |
 *       |  3D Warping  |
 *       |______________|
 *              |
 *   __________\|/__________
 *  |           |           |
 *  |    Left   |   Right   |------------ OUTPUT
 *  |___________|___________|
 ******************************************************************************/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm> //min,max

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_image.h"

//#define USE_LIBVLC 1

#include "sdl_aux.h"

#define PI 3.141592653589793
#define GAUSSIAN_KERNEL_SIZE 9
#define RADDEG  57.29577951f

struct user_params
{
  char *file_path;

  /* Screen related params */
  int screen_width;
  int screen_height;
  int screen_bpp;
  bool fullscreen;

  /* DIBR related params */
  bool hole_filling;
  bool paused;
  bool depth_filter;
  bool show_all;
  bool enable_occlusion_layer;
  double eye_sep;
  int ghost_threshold;
  bool enable_ghost;
  bool enable_splat;

  /* Gaussian filter parameters */
  double sigmax;
  double sigmay;
};

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

/****************** 3D Warping related functions ******************************/
double find_shiftMC3( user_params &p,
                   int depth, int Ny)
{
  int h;
  int nkfar = 128, nknear = 128, kfar = 0, knear = 0;
  int n_depth = 256;  // Number of depth planes
  int eye_sep = p.eye_sep;    // eye separation 6cm

  // This is a display dependant parameter and the maximum shift depends
  // on this value. However, the maximum disparity should not exceed
  // particular threshold, defined by phisiology of human vision.
  int Npix = 100; // 300 TODO: Make this adjustable in the interface.
  int h1 = 0;
  int A = 0;
  int h2 = 0;

  // Viewing distance. Usually 300 cm
  int D = 300;

  // According to N8038
  knear = nknear / 64;
  kfar = nkfar / 16;

  // Assumption 1: For visual purposes
  // This can be let as it is. However, then we are not able to have a
  // clear understanding of how the rendered views look like.
  // knear = 0 means everything is displayed behind the screen
  // which is practically not the case.
  // Interested user can remove this part to see what happens.
  knear = 0;
  A  = depth*(knear + kfar)/(n_depth-1);
  h1 = -eye_sep*Npix*( A - kfar )/D;
  h2 = (h1/2) % 1; //  Warning: Previously this was rem(h1/2,1)

  if (h2>=0.5)
    h = ceil(h1/2);
  else
    h = floor(h1/2);
  if (h<0)
    // It will never come here due to Assumption 1
    h = 0 - h;
  return h;
}

bool is_ghost(SDL_Surface *depth_map, int x, int y, int ghost_threshold)
{
  int SUM = 0;
  int Dxy = 0;
  int R = 1;

  for (int i = x-R; i <= x+R; i++)
    for(int j = y-R; j <= y+R; j++)
      if (i >= 0 && i < depth_map->w &&
          j >= 0 && j < depth_map->h)
      {

        Uint32 pixel = sdl_get_pixel(depth_map, i, j);
        Uint8 r, g, b;
        SDL_GetRGB (pixel, depth_map->format, &r, &g, &b);
        int Y, U, V;
        get_YUV(r, g, b, Y, U, V);

        if (i == x && j == y)
          Dxy = Y;

        SUM += Y;
      }


//  printf ("%f ", (SUM - (pow(R, 2) + 1) * Dxy));

//  if (SUM - ((pow(R, 2) + 1)*Dxy) >= ghost_threshold)
  if (SUM - 9*Dxy > ghost_threshold)
    return true;

  return false;
}


bool shift_surface ( user_params &p,
                     SDL_Surface *image_color,
                     SDL_Surface *depth_frame,
                     SDL_Surface *depth_frame_filtered,
                     SDL_Surface *left_image,
                     SDL_Surface *right_image,
                     bool hole_filling,
                     bool **mask_left,
                     bool **mask_right,
                     int S = 58 )
{
  // This value is half od the maximun shift
  // Maximun shift comes at depth == 0
  int N = 256; // Number of depth-planes
  // int S = 58;
  double depth_shift_table_lookup[N];
  int cols = image_color->w, rows = image_color->h;

  // \fixme remove from here
  for(int i = 0; i < N; i++)
    depth_shift_table_lookup[i] = find_shiftMC3(p, i, N);

  if (p.depth_filter)
  {
    filter_depth( depth_frame,
                  depth_frame_filtered,
                  cols, rows,
                  depth_frame->w, depth_frame->h);
  };

  bool **mask = mask_left;
  // Calculate left image
  for (int y = 0; y < rows; y++)
  {
    for (int x = cols-1; x >= 0; --x)
    {
      // get depth
      // depth_frame_filtered after filter applied FCI
      Uint32 pixel = sdl_get_pixel(depth_frame_filtered, x, y);
      Uint8 r, g, b;
      SDL_GetRGB (pixel, depth_frame_filtered->format, &r, &g, &b);
      int Y, U, V;
      get_YUV(r, g, b, Y, U, V);
      int D = Y;
      double shift = depth_shift_table_lookup [D];

      pixel = sdl_get_pixel(image_color, x, y);
      SDL_GetRGB (pixel, image_color->format, &r, &g, &b);

      if (r == 0 && g == 0 && b == 0)
        continue;

      if ( p.enable_ghost &&
           is_ghost(depth_frame_filtered, x, y, p.ghost_threshold))
        continue;

      if( x + S - shift < cols &&
          x + S - shift >= 0)
      {
        sdl_put_pixel( left_image, x + S-shift, y,
                       sdl_get_pixel (image_color, x, y) );

        mask [(int)(x+S-shift)][y] = 1;
      }

      if (p.enable_splat)
      {
        if( x + S - shift + 0.5 < cols &&
            x + S - shift + 0.5 >= 0)
        {
          sdl_put_pixel( left_image, x + S-shift + 0.5, y,
                         sdl_get_pixel (image_color, x, y) );
          mask [(int)(x+S-shift + 0.5)][y] = 1;
        }

        if( x + S - shift - 0.5 < cols &&
            x + S - shift - 0.5 >= 0)
        {
          sdl_put_pixel( left_image, x + S-shift - 0.5, y,
                       sdl_get_pixel (image_color, x, y) );
          mask [(int)(x+S-shift - 0.5)][y] = 1;
        }
      }
    }

    if(hole_filling)
    {
      for (int x = 1; x < cols; x++)
      {
        if ( mask[x][y] == 0 )
        {
          if ( x - 7 < 0)
          {
            sdl_put_pixel (left_image, x, y, sdl_get_pixel(image_color, x, y));
          }
          else
          {
            sdl_put_pixel (left_image, x, y, sdl_get_pixel(left_image, x-1, y));
          }
        }
      }
    }
  }

  mask = mask_right;
  // Calculate right image
  for (int y = 0; y < rows; y++)
  {
    for (int x = 0; x < cols; x++)
    {
      // get depth
      // depth_frame_filtered after filter FCI
      Uint32 pixel = sdl_get_pixel(depth_frame_filtered, x, y);
      Uint8 r, g, b;
      SDL_GetRGB (pixel, depth_frame_filtered->format, &r, &g, &b);
      int Y, U, V;
      get_YUV(r, g, b, Y, U, V);
      int D = Y;
      double shift = depth_shift_table_lookup [D];

      if ( p.enable_ghost &&
           is_ghost(depth_frame_filtered, x, y, p.ghost_threshold))
        continue;

      pixel = sdl_get_pixel(image_color, x, y);
      SDL_GetRGB (pixel, image_color->format, &r, &g, &b);

      if (r == 0 && g == 0 && b == 0)
        continue;

      if( ((x + shift - S) >= 0) &&
          ((int)(x + shift - S) < cols))
      {
        sdl_put_pixel ( right_image, x + shift - S, y,
                        sdl_get_pixel (image_color, x, y) );
        mask [(int)(x+shift-S)][y] = 1;
      }

      if (p.enable_splat)
      {
        if( x + shift - S - 0.5 >= 0 &&
            x + shift - S + 0.5 < cols)
        {
          sdl_put_pixel ( right_image, x + shift - S - 0.5, y,
                          sdl_get_pixel (image_color, x, y) );
          mask [(int)(x+shift-S - 0.5)][y] = 1;
        }

        if( x + shift - S + 0.5 >= 0 &&
            x + shift - S + 0.5 < cols)
        {
          sdl_put_pixel ( right_image, x + shift - S + 0.5, y,
                          sdl_get_pixel (image_color, x, y) );
          mask [(int)(x+shift-S + 0.5)][y] = 1;
        }

      }
    }

    if(hole_filling)
    {
      for (int x = cols-1 ; x >= 0; --x)
      {
        if ( mask[x][y] == 0 )
        {
          if ( x + 7 > cols - 1)
          {
            sdl_put_pixel (right_image, x, y, sdl_get_pixel(image_color, x, y));
          }
          else
          {
            sdl_put_pixel (right_image, x, y, sdl_get_pixel(right_image, x+1, y));
          }
        }
      }
    }

  }
  return true;
}

/********************* VLC related functions **********************************/
struct vlc_sdl_ctx
{
  SDL_Surface *surf;
  SDL_mutex *mutex;


  //int frame_start_time;
  //int frame_current_time;
  //int frame_count ;
};




/*
 * Initializes SDL, OpenGL and video window.
 */
bool init(user_params &p)
{
  Uint32 rmask, gmask, bmask, amask;

  /* initialize SDL */
  if( SDL_Init(SDL_INIT_EVERYTHING) < 0 )
    return false;


  /* ctx initialization */

  sdl_get_pixel_mask(rmask, gmask, bmask, amask);

  return true;
}

/* Removes objects before closes */
void clean_up()
{

  /* Stop SDL */
  SDL_Quit();
}

void set_default_params(user_params &p)
{
  p.screen_width = 1920;
  p.screen_height = 1080;
  p.screen_bpp = 32;
  p.fullscreen = false;

  p.hole_filling = false;
  p.paused = true;
  p.depth_filter = true;
  p.show_all = false;
  p.enable_occlusion_layer = false;
  p.eye_sep = 6;
  p.enable_ghost = false;
  p.ghost_threshold = 20;
  p.enable_splat = true;

  /* Gaussian filter parameters */
  p.sigmax = 500.0;
  p.sigmay = 10; /* assymetric gaussian filter */
}

int main(int argc, char* argv[])
{
  user_params p, p2;
  struct vlc_sdl_ctx ctx;

  bool quit = false;
  SDL_Event event;

  int S = 20; //58;

  // Handling parameters
  set_default_params(p);
  //TODO: parse_opts(argc, argv);

  if(argc < 2)
  {
      printf("Usage: %s <filename>\n", argv[0]);
      return EXIT_FAILURE;
  }

  if(argc >2)
    p2.file_path = argv[2];
  else
    p2.file_path = "stereo.bmp";

  p.file_path = argv[1]; // path to video/image.

  if(init(p) == false)
    return 1;

  if ( ( SDL_EnableKeyRepeat( 100, SDL_DEFAULT_REPEAT_INTERVAL ) ) )
  {
    fprintf( stderr, "Setting keyboard repeat failed: %s\n",
             SDL_GetError( ) );
    exit( 1 );
  }
  SDL_Surface *image = IMG_Load(p.file_path);

  // SDL_Surface *image_2 = IMG_Load("samples/images/stilllife.jpg");
  SDL_Surface *image_all = sdl_create_RGB_surface(image, image->w, image->h*2 );
  SDL_Surface *image_color = sdl_create_RGB_surface(image, image->w/2, image->h/2 );
  sdl_crop_surface( image, image_color, 0, 0, image->w/2, image->h/2 );

  SDL_Surface *depth_frame = sdl_create_RGB_surface( image, image->w/2, image->h/2 );
  sdl_crop_surface( image, depth_frame, image->w/2, 0, image->w/2, image->h/2 );

  SDL_Surface *depth_frame_filtered = sdl_create_RGB_surface( image, image->w/2, image->h/2 );
  sdl_crop_surface( image, depth_frame_filtered, image->w/2, 0, image->w/2, image->h/2 );

  SDL_Surface *occlusion_color_frame = sdl_create_RGB_surface( image, image->w/2, image->h/2 );
  sdl_crop_surface( image, occlusion_color_frame, 0, image->h/2, image->w/2, image->h/2 );

  SDL_Surface *occlusion_depth_frame = sdl_create_RGB_surface( image, image->w/2, image->h/2 );
  sdl_crop_surface( image, occlusion_depth_frame, image->w/2, image->h/2, image->w/2, image->h/2 );

/*
  SDL_Surface *image_all_2 = sdl_create_RGB_surface(image_2, image_2->w, image_2->h*2 );
  SDL_Surface *image_color_2 = sdl_create_RGB_surface(image_2, image_2->w/2, image_2->h);
  sdl_crop_surface( image_2, image_color_2, 0, 0, image_2->w/2, image_2->h );

  SDL_Surface *depth_frame_2 = sdl_create_RGB_surface(image_2, image_2->w/2, image_2->h);
  sdl_crop_surface( image_2, depth_frame_2, image_2->w/2, 0, image_2->w/2, image_2->h); */

  /* Create a 32-bit surface with the bytes of each pixel in R,G,B,A order, as
   * expected by OpenGL for textures */
  SDL_Surface *left_color = sdl_create_RGB_surface(image_color, image_color->w, image_color->h);
  SDL_Surface *right_color = sdl_create_RGB_surface( image_color, image_color->w, image_color->h);
  SDL_Surface *stereo_color = sdl_create_RGB_surface( image_color, image->w, image->h/2);


  int Mode = GL_RGB;
  if(image->format->BytesPerPixel == 4)
    Mode = GL_RGBA;

  calc_gaussian_kernel(p.sigmax, p.sigmay);

//  ctx.frame_count = 0;

  // allocate mask matrix
  int cols = image_color->w;
  int rows = image_color->h;
  bool **mask_left = (bool **)malloc(cols * sizeof(bool*));
  bool **mask_right = (bool **)malloc(cols * sizeof(bool*));
  for (int i = 0; i < cols; i++)
  {
    mask_left[i] = (bool*) malloc(rows * sizeof(bool));
    mask_right[i] = (bool*) malloc(rows * sizeof(bool));
  }


  /****** Main Loop ******/
//    SDL_LockMutex(ctx.mutex);
    sdl_crop_surface( image, image_color, 0, 0, image->w/2, image->h/2);
    sdl_crop_surface( image, depth_frame, image->w/2, 0, image->w/2, image->h/2);
    sdl_crop_surface( image, occlusion_color_frame, 0, image->h/2, image->w/2, image->h/2);
    sdl_crop_surface( image, occlusion_depth_frame, image->w/2, image->h/2, image->w/2, image->h/2);

    // SDL_BlitSurface(image_color_2, NULL, image_color, NULL);
    // SDL_BlitSurface(depth_frame_2, NULL, depth_frame, NULL);

    //FCI just have a copy to work with
    sdl_crop_surface( image,
                      depth_frame_filtered,
                      image->w/2,
                      0,
                      image->w/2,
                      image->h );
  //  SDL_UnlockMutex(ctx.mutex);

    // Generate stereo image
    // FCI depth_frame_filtered
    SDL_FillRect(left_color, NULL, 0xFFFFFF);
    SDL_FillRect(right_color, NULL, 0xFFFFFF);

    for (int i = 0; i < cols; i++)
    {
      memset (mask_left[i], false, rows*sizeof(bool));
      memset (mask_right[i], false, rows*sizeof(bool));
    }
    if (p.enable_occlusion_layer)
    {
      shift_surface( p,
                     occlusion_color_frame,
                     occlusion_depth_frame,
                     occlusion_depth_frame,
                     left_color,
                     right_color,
                     false,
                     mask_left,
                     mask_right,
                     S );
    }

    shift_surface( p, image_color, depth_frame,
                   depth_frame_filtered,
                   left_color,
                   right_color,
                   p.hole_filling,
                   mask_left,
                   mask_right,
                   S );

    SDL_FillRect(stereo_color, NULL, 0x000000);
    SDL_BlitSurface (left_color, NULL, stereo_color, NULL);

    SDL_Rect dest;
    dest.x = left_color->w;
    dest.y = 0;
    dest.w = right_color->w;
    dest.h = right_color->h;
    SDL_BlitSurface (right_color, NULL, stereo_color, &dest);

    // Compose original and depth
    SDL_BlitSurface (stereo_color, NULL, image_all, NULL);
    dest.x = 0;
    dest.y = stereo_color->h;
    SDL_BlitSurface (image_color, NULL, image_all, &dest);
    dest.x = image_color->w;
    SDL_BlitSurface (depth_frame_filtered, NULL, image_all, &dest);

    SDL_Surface *surface_to_display = stereo_color;
    SDL_GL_SwapBuffers();


  SDL_SaveBMP(stereo_color, p2.file_path);

  // Clean up everything
  clean_up();

  return 0;
}

