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
#ifndef __SDL_AUX_H__
#define __SDL_AUX_H__
#include <stdio.h>
#include <stdlib.h>

#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL/SDL_image.h"

// need to create get_RGB(oposite to get_YUV)  if going to put back in
// image_depth2 FCI****
// need to store values of UV if goint to put pixel back 
// get precise constants FCI***
void get_RGB( int Y, int U, int V, Uint8 &r, Uint8 &g, Uint8 &b);

void get_YUV(Uint8 r, Uint8 g, Uint8 b, int &Y, int &U, int &V);

/*
 * SDL interprets each pixel as a 32-bit number, so our masks must depend on the
 * endianness (byte order) of the machine.
 */
void sdl_get_pixel_mask( Uint32 &rmask,
                         Uint32 &gmask,
                         Uint32 &bmask,
                         Uint32 &amask );

SDL_Surface* sdl_crop_surface( SDL_Surface* orig,
                           SDL_Surface* dest,
                           short int x, short int y,
                           short unsigned int width, short unsigned int height );

Uint32 sdl_get_pixel(SDL_Surface *surface, int x, int y);

void sdl_put_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

/*
 * Creates an RGB surface using the same parameters as surf. Width and height,
 * however, can be different.
 */
SDL_Surface *sdl_create_RGB_surface(SDL_Surface *surf, int w, int h);
#endif
