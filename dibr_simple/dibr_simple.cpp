#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <math.h>

#define MAX_ROWS 4096
#define MAX_COLS 4096

using namespace std;

struct image
{
  int width;
  int height;
  int R;
  int G;
  int B;
  int depth;
  int shift_num;
  int priority;
  int on;
};

image** create_image_array(unsigned int height, unsigned int width) {
    image** image_in;
     image_in = new image*[height];
        for(unsigned int i = 0; i < height; i++) {
            image_in[i] = new image[width];
    }
    return image_in;
}

int get_depth(const char *fname_d);
void depth_section();
void shift();

image image_in[MAX_ROWS][MAX_COLS];
image image_shift[MAX_ROWS][MAX_COLS];  // every set has R,G,B  example image[0][0].R, image[0][0].G, image[0][0].B;//
image image_size;

unsigned int  width = 0, height = 0;   // image width, image height
int dep_max = 0 ,dep_min = 0;

unsigned char* read_raw(const char *fname_s, unsigned int& rgb_raw_data_offset ) {
    FILE          *fp_s = NULL;    // source file handler
    unsigned char *image_s = NULL; // source image array

    fp_s = fopen(fname_s, "rb");   // source file handler
    if (fp_s == NULL) {
        printf("fname_s=%s\n", fname_s);
        perror("fopen fp_s error\n");
      return NULL;
    }

    // move offset to 10 to find rgb raw data offset
    fseek(fp_s, 10, SEEK_SET);
    fread(&rgb_raw_data_offset, sizeof(unsigned int), 1, fp_s);
    // move offset to 18    to get width & height;
    fseek(fp_s, 18, SEEK_SET);
    fread(&width,  sizeof(unsigned int), 1, fp_s);
    fread(&height, sizeof(unsigned int), 1, fp_s);
    // move offset to rgb_raw_data_offset to get RGB raw data
    fseek(fp_s, rgb_raw_data_offset, SEEK_SET);


    image_s = (unsigned char *)malloc((size_t)width * height * 3);   //array
    if (image_s == NULL) {
      printf("malloc images_s error\n");
      return NULL;
    }
    fread(image_s, sizeof(unsigned char), (size_t)(long)width * height * 3, fp_s);
    fclose(fp_s);
    return image_s;
}

unsigned char* gen_header(int height, int width, int file_size) {
    static unsigned char header[54] = {
      0x42,        // identity : B
      0x4d,        // identity : M
      0, 0, 0, 0,  // file size
      0, 0,        // reserved1
      0, 0,        // reserved2
      54, 0, 0, 0, // RGB data offset
      40, 0, 0, 0, // struct BITMAPINFOHEADER size
      0, 0, 0, 0,  // bmp width
      0, 0, 0, 0,  // bmp height
      1, 0,        // planes
      24, 0,       // bit per pixel
      0, 0, 0, 0,  // compression
      0, 0, 0, 0,  // data size
      0, 0, 0, 0,  // h resolution
      0, 0, 0, 0,  // v resolution
      0, 0, 0, 0,  // used colors
      0, 0, 0, 0   // important colors
    };

     header[2] = (unsigned char)(file_size & 0x000000ff);
     header[3] = (file_size >> 8)  & 0x000000ff;
     header[4] = (file_size >> 16) & 0x000000ff;
     header[5] = (file_size >> 24) & 0x000000ff;


     // width
     header[18] = width & 0x000000ff;
     header[19] = (width >> 8)  & 0x000000ff;
     header[20] = (width >> 16) & 0x000000ff;
     header[21] = (width >> 24) & 0x000000ff;


     // height
     header[22] = height &0x000000ff;
     header[23] = (height >> 8)  & 0x000000ff;
     header[24] = (height >> 16) & 0x000000ff;
     header[25] = (height >> 24) & 0x000000ff;

    return header;
}

void load_image_in(const char *fname_s, unsigned int& rgb_raw_data_offset )
{
    unsigned char* image_s = read_raw(fname_s, rgb_raw_data_offset);
    for(unsigned int j = 0; j < width; j++) {
      for(unsigned int i = 0; i < height; i++) {
          int idx = width * i + j;
          int idx2 = 3 * idx;
         image_in[i][j].R = *(image_s + idx2 + 2);
         image_in[i][j].G = *(image_s + idx2 + 1);
         image_in[i][j].B = *(image_s + idx2 + 0);
      }
    }
    free(image_s);


}

int get_raw(const char *fname_s, const char *fname_t) {
    FILE          *fp_t = NULL;    // target file handler
    unsigned char *image_t = NULL; // target image array
    unsigned int file_size = 0;           // file size
    unsigned int rgb_raw_data_offset = 0; // RGB raw data offset

    image_t = (unsigned char *)malloc((size_t)width * height * 3);   //array
    if (image_t == NULL) {
      printf("malloc image_t error\n");
      return -1;
    }
    load_image_in(fname_s, rgb_raw_data_offset);
    depth_section();
    shift();

    for(unsigned int j = 0; j < width; j++) {
      for(unsigned int i = 0; i < height; i++) {
          int idx = width * i + j;
          int idx2 = 3 * idx;
        *(image_t + idx2 + 2) = image_shift[i][j].R;
        *(image_t + idx2 + 1) = image_shift[i][j].G;
        *(image_t + idx2 + 0) = image_shift[i][j].B;
      }
    }


    // write to new bmp
    fp_t = fopen(fname_t, "wb");
    if (fp_t == NULL) {
        printf("fname_t=%s\n", fname_t);
        perror("fopen fname_t error\n");
        return -1;
    }


     // file size
     file_size = width * height * 3 + rgb_raw_data_offset;
     unsigned char* header = gen_header(height, width, file_size);
     // write header
     fwrite(header, sizeof(unsigned char), rgb_raw_data_offset, fp_t);
     // write image
     fwrite(image_t, sizeof(unsigned char), (size_t)(long)width * height * 3, fp_t);
     free(image_t);
     fclose(fp_t);
     return 0;
}


int get_depth(const char *fname_d)
{
     FILE          *fp_d = NULL;    // depth file handler
     unsigned char *image_d = NULL;   //depth image array
     unsigned int depth_raw_data_offset;
     float dep_r=0, dep_g=0, dep_b=0;
     dep_max =-128;
     dep_min =127;

     fp_d = fopen(fname_d, "rb");
     if (fp_d == NULL) {
        printf("fname_d=%s\n", fname_d);
        perror("fopen fp_d error\n");
        return -1;
     }
     // move offset to 10 to find rgb raw data offset
     fseek(fp_d, 10, SEEK_SET);
     fread(&depth_raw_data_offset, sizeof(unsigned int), 1, fp_d);
     // move offset to 18    to get width & height;
     fseek(fp_d, 18, SEEK_SET);
     fread(&width,  sizeof(unsigned int), 1, fp_d);
     fread(&height, sizeof(unsigned int), 1, fp_d);
     // move offset to rgb_raw_data_offset to get RGB raw data
     fseek(fp_d, depth_raw_data_offset, SEEK_SET);

     image_d = (unsigned char *)malloc((size_t)width * height * 3);
     if (image_d == NULL) {
         printf("malloc images_d error\n");
         return -1;
     }

     fread(image_d, sizeof(unsigned char), (size_t)(long)width * height * 3, fp_d);

     for(unsigned int y = 0; y != width; ++y) {
         for(unsigned int x = 0; x != height; ++x) {
             int idx = width * x + y;
            int idx2 = 3 * idx;
             dep_r = *(image_d + idx2 + 2);
             dep_g = *(image_d + idx2 + 1);
             dep_g = *(image_d + idx2 + 0);
             image_in[x][y].depth = (dep_r + dep_g + dep_b)/3;
             if(image_in[x][y].depth > dep_max)
        dep_max = image_in[x][y].depth;
         else if(image_in[x][y].depth < dep_min)
                dep_min = image_in[x][y].depth;
              }
    }
    fclose(fp_d);
    free(image_d);
    return 0;
}

void depth_section()
{
    int section;
    section = (dep_max - dep_min + 1) / 8;
    printf("%d\n", section);
    for(unsigned int i=0; i!=height; ++i) {
        for(unsigned int j=0; j!=width; ++j) {
          //--------------------shift-8-------------------------------------------
            if (image_in[i][j].depth > dep_min)
                continue;
            int shift_num = int((dep_min - image_in[i][j].depth)/section+0.5);
              image_in[i][j].shift_num = shift_num;
              image_in[i][j].priority =  shift_num;

        }
     }
}


void shift()   // i = row, j = column
{
    //int pointer = 0;
    //float length = 0;
    //float counter = 0, son = 1, mom = 0;
    float temp_r_l, temp_r_r;
    float temp_g_l, temp_g_r;
    float temp_b_l, temp_b_r;
    int save_j=0;
    //int direction=0;
    for(unsigned int i=0 ; i<=height; i++)
        for(unsigned int j=0; j<=(width-1); j++)  //(image_size.width-1)
        {
            int shift=image_in[i][j].shift_num;
            int j1 = j+shift;
            if(image_shift[i][j1].on == 0 || (image_in[i][j].priority > image_shift[i][j1].priority))
            {
                image_shift[i][j1].R = image_in[i][j].R;
                image_shift[i][j1].G = image_in[i][j].G;
                image_shift[i][j1].B = image_in[i][j].B;
                image_shift[i][j1].on = 1;
                image_shift[i][j1].priority = image_in[i][j].priority;
            }
        }// end of switch


    for(int m = 0; m<width; m++)
        for(int n = 0; n<height; n++)
        {
            if(image_shift[m][n].on == 0 && image_shift[m][n - 1].on == 1 && image_shift[m][n + 1].on == 1)
            {
                image_shift[m][n].R = image_shift[m][n + 1].R;
                image_shift[m][n].G = image_shift[m][n + 1].G;
                image_shift[m][n].B = image_shift[m][n + 1].B;
                image_shift[m][n].on = 1;
            }


            if(image_shift[m][n].on == 0 && image_shift[m + 1][n].on == 1 && image_shift[m - 1][n].on == 1)
            {
                image_shift[m][n].R = image_shift[m + 1][n].R;
                image_shift[m][n].G = image_shift[m + 1][n].G;
                image_shift[m][n].B = image_shift[m + 1][n].B;
                image_shift[m][n].on = 1;
            }
        }


  for(int i=0; i<width; i++)
     for(int j=0; j<height; j++)
     {
        int k = 0;
        if(j == 0)
        {
          while(image_shift[i][k].on == 0)
                k++;
          for(int m = 0; m < k; m++)
          {
             image_shift[i][m].R = image_shift[i][k].R;
             image_shift[i][m].G = image_shift[i][k].G;
             image_shift[i][m].B = image_shift[i][k].B;
          }
          j = k;
        }


        save_j = j;


        while(image_shift[i][save_j].on == 0 )
        {
           if(image_shift[i][save_j - 1].on == 1)
           {
              temp_r_l = image_shift[i][save_j - 1].R;
              temp_g_l = image_shift[i][save_j - 1].G;
              temp_b_l = image_shift[i][save_j - 1].B;
           }
           if(image_shift[i][save_j + 1].on == 1)
           {
              temp_r_r = image_shift[i][save_j + 1].R;
              temp_g_r = image_shift[i][save_j + 1].G;
              temp_b_r = image_shift[i][save_j + 1].B;
           }
           save_j++;
           //counter++;
        }
// ---------------------- intepolation filling -----------------------------------------
//interpolation_filling:
        //save_j = j;
        //while( image_shift[i][save_j].on == 0 )
        //{
           //mom = counter + 1;
           //for(int k = counter; k > 0; k--)
           //{
              //float test_r = 0, test_b = 0, test_g = 0;
              //test_r = ((float)son/(float)mom)*temp_r_r + ((float)(mom - son )/(float)mom)*temp_r_l;
              //image_shift[i][save_j].R = test_r;
              //test_g = ((float)son/(float)mom)*temp_g_r + ((float)(mom - son )/(float)mom)*temp_g_l;
              //image_shift[i][save_j].G = test_g;
              //test_b = ((float)son/(float)mom)*temp_b_r + ((float)(mom - son )/(float)mom)*temp_b_l;
              //image_shift[i][save_j].B = test_b;
              //son++;
              //save_j++;
           //}
        //}
        //counter = 0;
        //son = 1;
        //mom = 0;
    }

} // end of shift

int main()
{
int totalNumbers=1;
    char FileName[100], OutName[100], Dep[100];//百圖檔名用

    for(int Number=1; Number<=totalNumbers; Number++)
    {
        sprintf(Dep, "../images/%ddep.bmp", Number);
        sprintf(FileName, "../images/%d.bmp", Number);
        sprintf(OutName, "%d_l2.bmp", Number);
        get_depth(Dep);
        get_raw(FileName, OutName);
    }
    //system("pause");
}
