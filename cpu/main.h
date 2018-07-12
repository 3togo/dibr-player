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
