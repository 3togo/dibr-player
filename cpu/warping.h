/****************** 3D Warping related functions ******************************/
double find_shiftMC3( user_params &p,
                   int depth, int Ny);

bool is_ghost(SDL_Surface *depth_map, int x, int y, int ghost_threshold);

bool shift_surface ( user_params &p,
                     SDL_Surface *image_color,
                     SDL_Surface *depth_frame,
                     SDL_Surface *depth_frame_filtered,
                     SDL_Surface *left_image,
                     SDL_Surface *right_image,
                     bool hole_filling,
                     bool **mask_left,
                     bool **mask_right,
                     int S = 58 );
