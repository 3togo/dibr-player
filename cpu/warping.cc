#include "main.h"
#include <math.h>
#include <algorithm> //min,max
#include "sdl_aux.h"
#include "filters.h"

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
/*
            Uint32 r_sum = 0, g_sum = 0, b_sum = 0;
            for (int x1 = x-7; x1 <= x-4; x1++)
            {
              Uint32 pixel = sdl_get_pixel(left_image, x1, y);
              Uint8 r, g, b;
              SDL_GetRGB (pixel, left_image->format, &r, &g, &b);
              r_sum += r;
              g_sum += g;
              b_sum += b;
            }

            Uint8 r_new, g_new, b_new;
            r_new = (Uint8)(r_sum / 4);
            g_new = (Uint8)(g_sum / 4);
            b_new = (Uint8)(b_sum / 4);
            Uint32 pixel_new = SDL_MapRGB(left_image->format,
                                          r_new,
                                          g_new,
                                          b_new);
            sdl_put_pixel (left_image, x, y, pixel_new ); */
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
/*
            Uint32 r_sum = 0, g_sum = 0, b_sum = 0;
            for (int x1 = x+4; x1 <= x+7; x1++)
            {
              Uint32 pixel = sdl_get_pixel(right_image, x1, y);
              Uint8 r, g, b;
              SDL_GetRGB (pixel, right_image->format, &r, &g, &b);
              r_sum += r;
              g_sum += g;
              b_sum += b;
            }

            Uint8 r_new, g_new, b_new;
            r_new = (Uint8)(r_sum / 4);
            g_new = (Uint8)(g_sum / 4);
            b_new = (Uint8)(b_sum / 4);
            Uint32 pixel_new = SDL_MapRGB(  right_image->format,
                                            r_new,
                                            g_new,
                                            b_new );
            sdl_put_pixel (right_image, x, y, pixel_new ); */
          }
        }
      }
    }

  }
  return true;
}
