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

#define USE_LIBVLC 1

#ifdef USE_LIBVLC
#include <vlc/vlc.h>
#endif
#include <iostream>
using namespace std;
#include "main.h"
#include "sdl_aux.h"
#include "filters.h"
#include "warping.h"
#define PI 3.141592653589793

#define RADDEG  57.29577951f


/********************* VLC related functions **********************************/
struct vlc_sdl_ctx
{
  SDL_Surface *surf;
  SDL_mutex *mutex;

#ifdef USE_LIBVLC
  libvlc_instance_t *libvlc;
  libvlc_media_t *m;
  libvlc_media_player_t *mp;
#endif

  int frame_start_time;
  int frame_current_time;
  int frame_count ;
};

#ifdef USE_LIBVLC
static void *lock(void *data, void **p_pixels)
{
  struct vlc_sdl_ctx *ctx = (struct vlc_sdl_ctx*)data;

  SDL_LockMutex(ctx->mutex);
  SDL_LockSurface(ctx->surf);
  *p_pixels = ctx->surf->pixels;

  return NULL; /* picture identifier, not needed here */
}

static void unlock(void *data, void *id, void *const *p_pixels)
{
  struct vlc_sdl_ctx *ctx = (struct vlc_sdl_ctx*)data;

  SDL_UnlockSurface(ctx->surf);
  SDL_UnlockMutex(ctx->mutex);

  assert(id == NULL); /* picture identifier, not needed here */
}

static void display(void *data, void *id)
{
  /* VLC wants to display the video */
  (void) data;
  assert(id == NULL);
}
#endif
/* end libVLC */

/******************** OpenGL related functions ********************************/
/**
 * Initializes Core OpenGL Features.
 */
bool opengl_init(user_params &p)
{
  glEnable( GL_TEXTURE_2D );

  glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
  glViewport( 0, 0, p.screen_width, p.screen_height);

  glClear( GL_COLOR_BUFFER_BIT );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  glOrtho(0.0f, p.screen_width, p.screen_height, 0.0f, -1.0f, 1.0f);

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  return true;
}

/*
 * Initializes SDL, OpenGL and video window.
 */
bool init(user_params &p, vlc_sdl_ctx &ctx)
{
  Uint32 rmask, gmask, bmask, amask;

  /* initialize SDL */
  if( SDL_Init(SDL_INIT_EVERYTHING) < 0 )
    return false;

  if (SDL_SetVideoMode( p.screen_width,
                        p.screen_height,
                        p.screen_bpp,
                        /* SDL_FULLSCREEN | */ SDL_OPENGL) == NULL )
    return false;

  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 ); // *new*

  if( opengl_init(p) == false )
    return false;

  SDL_WM_SetCaption("DIBR player -- Guassian Filter", NULL);

  /* ctx initialization */
  ctx.mutex = SDL_CreateMutex();
    
  sdl_get_pixel_mask(rmask, gmask, bmask, amask);
  ctx.surf  = SDL_CreateRGBSurface( SDL_SWSURFACE,
                                    p.screen_width,
                                    p.screen_height,
                                    32,
                                    rmask,
                                    gmask,
                                    bmask,
                                    0 );
   //puts("just doing SDL_CreateRGBSurface");
#ifdef USE_LIBVLC
  /* Initialize libVLC    */
  char const *vlc_argv[] =
  {
    "--no-audio", /* skip any audio track */
    "--no-xlib"
  };
  int vlc_argc = sizeof(vlc_argv) / sizeof(*vlc_argv);
  //ctx.libvlc = libvlc_new(vlc_argc, vlc_argv);
  ctx.libvlc = libvlc_new(0, NULL);
  if (ctx.libvlc ==NULL) {
    puts("Have you installed vlc player?");
    return false;
    }
  //cout << "eye_sep:" << ctx.libvlc << "|" << vlc_argc << "|" << vlc_argv[0]<< "|" << vlc_argv[1]  << endl;
  printf ("the video file is : %s\n", p.file_path);
  ctx.m = libvlc_media_new_path(ctx.libvlc, p.file_path);
  ctx.mp = libvlc_media_player_new_from_media(ctx.m);
  libvlc_media_release(ctx.m);
  libvlc_video_set_callbacks(ctx.mp, lock, unlock, display, &ctx);
  libvlc_video_set_format( ctx.mp,
                           "RGBA",
                           p.screen_width,
                           p.screen_height,
                           p.screen_width*4 );
#endif
  puts("done init");
  return true;
}

/* Removes objects before closes */
void clean_up(vlc_sdl_ctx &ctx)
{
#ifdef USE_LIBVLC
  /* Stop stream and clean up libVLC */
  libvlc_media_player_stop(ctx.mp);
  libvlc_media_player_release(ctx.mp);
  libvlc_release(ctx.libvlc);
#endif

  /* Close window and clean up libSDL */
  SDL_DestroyMutex(ctx.mutex);
  SDL_FreeSurface(ctx.surf);

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
  user_params p;
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

  p.file_path = argv[1]; // path to video/image.

  if(init(p, ctx) == false)
    return 1;

  if ( ( SDL_EnableKeyRepeat( 100, SDL_DEFAULT_REPEAT_INTERVAL ) ) )
  {
    fprintf( stderr, "Setting keyboard repeat failed: %s\n",
             SDL_GetError( ) );
    exit( 1 );
  }
#ifdef USE_LIBVLC
  SDL_Surface *image = ctx.surf;
#else
  SDL_Surface *image = IMG_Load(p.file_path);
#endif

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

  GLuint TextureID = 0;
  /* Generate the openGL textures */
  glEnable( GL_TEXTURE_2D );
  glGenTextures(1, &TextureID);
  glBindTexture(GL_TEXTURE_2D, TextureID);

  int Mode = GL_RGB;
  if(image->format->BytesPerPixel == 4)
    Mode = GL_RGBA;

  calc_gaussian_kernel(p.sigmax, p.sigmay);

#ifdef USE_LIBVLC
  libvlc_media_player_play(ctx.mp);
#endif
  ctx.frame_count = 0;

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
  while(quit == false)
  {
    SDL_LockMutex(ctx.mutex);
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
    SDL_UnlockMutex(ctx.mutex);

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
    if (p.show_all)
      surface_to_display = image_all;

    glTexImage2D( GL_TEXTURE_2D,
                  0, Mode,
                  surface_to_display->w, surface_to_display->h, 0,
                  Mode, GL_UNSIGNED_BYTE, surface_to_display->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /****** Get Most Current Time ******/
    ctx.frame_start_time = SDL_GetTicks();

    /****** Draw Rectangle ******/
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    // Bind the texture to which subsequent calls refer to
    glBindTexture( GL_TEXTURE_2D, TextureID);
      glBegin( GL_QUADS );
      //Bottom-left vertex (corner)
      glTexCoord2i( 0, 0 );
      glVertex3f( 0.f, 0.f, 0.0f );
      //Bottom-right vertex (corner)
      glTexCoord2i( 1, 0 );
      glVertex3f( p.screen_width, 0.f, 0.f );
      //Top-right vertex (corner)
      glTexCoord2i( 1, 1 );
      glVertex3f( p.screen_width, p.screen_height, 0.f );
      //Top-left vertex (corner)
      glTexCoord2i( 0, 1 );
      glVertex3f( 0.f, p.screen_height, 0.f );
    glEnd();

    /****** Check for Key & System input ******/
    while(SDL_PollEvent(&event))
    {
      /******  Application Quit Event ******/
      switch (event.type)
      {
        case SDL_KEYDOWN:
          if(event.key.keysym.sym == 27)
            quit = true;
          else if(event.key.keysym.sym == '=')
            p.eye_sep++;
          else if(event.key.keysym.sym == '-')
            p.eye_sep--;
          else if(event.key.keysym.sym == 'h')
          {
            p.hole_filling = !p.hole_filling;
            printf ("Hole filling: %d.\n", p.hole_filling);
          }
          else if(event.key.keysym.sym == 'f')
          {
            p.depth_filter = !p.depth_filter;     // filter toggle FCI****
            printf ("Depth filter: %d.\n", p.depth_filter);
          }
          else if(event.key.keysym.sym == SDLK_SPACE)
          {
            p.paused = !p.paused;
#ifdef USE_LIBVLC
            libvlc_media_player_set_pause(ctx.mp, p.paused);
#endif
          }
          else if(event.key.keysym.sym == 'a')
          {
            p.show_all = !p.show_all;
            printf ("Show reference frames: %d.\n", p.show_all);
          }
          else if(event.key.keysym.sym == 'o')
          {
            p.enable_occlusion_layer = !p.enable_occlusion_layer;
            printf ("Occlusion Layer: %d.\n", p.enable_occlusion_layer);
          }
          else if(event.key.keysym.sym == 'g')
          {
            p.enable_ghost = !p.enable_ghost;
            printf ("Enable ghost: %d.\n", p.enable_ghost);
          }
          else if(event.key.keysym.sym == 's')
          {
            p.enable_splat = !p.enable_splat;
            printf ("Enable splat: %d.\n", p.enable_ghost);
          }

          break;
      }
    }

    /****** Update Screen And Frame Counts ******/
    SDL_GL_SwapBuffers();
    ctx.frame_count++;
    ctx.frame_current_time = SDL_GetTicks();

    /****** Frame Rate Handle ******/
    if((ctx.frame_current_time - ctx.frame_start_time) < (1000/60))
    {
      ctx.frame_count = 0;
      SDL_Delay((1000/60) - (ctx.frame_current_time - ctx.frame_start_time));
    }
  }

  // Clean up everything
  clean_up(ctx);

  return 0;
}

