#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <math.h>

#define Width 576
#define Height 720
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

void depth_section();
void shift();
image image_in_array[4096*4096];
image* image_in= (image *)malloc(4096 * 4096);

image image_shift[4096][4096];  // every set has R,G,B  example image[0][0].R, image[0][0].G, image[0][0].B;
image image_size;


char ch = '\0', depth = '\0';
unsigned int  width = 0, height = 0;   // image width, image height
int dep_max = 0, dep_min = 0;

unsigned char* read_bmp(const char *fname_s, unsigned int* rgb_raw_data_offset_ptr ) {
    FILE          *fp_s = NULL;    // source file handler
    unsigned char *image_s = NULL; // source image array

    fp_s = fopen(fname_s, "rb");   // source file handler

    if (fp_s == NULL) {
      printf("fopen fp_s error\n");
      return NULL;
    }

    // move offset to 10 to find rgb raw data offset
    fseek(fp_s, 10, SEEK_SET);
    fread(rgb_raw_data_offset_ptr, sizeof(unsigned int), 1, fp_s);
    // move offset to 18    to get width & height;
    fseek(fp_s, 18, SEEK_SET);
    fread(&width,  sizeof(unsigned int), 1, fp_s);
    fread(&height, sizeof(unsigned int), 1, fp_s);
    // move offset to rgb_raw_data_offset to get RGB raw data
    fseek(fp_s, *rgb_raw_data_offset_ptr, SEEK_SET);

    image_s = (unsigned char *)malloc((size_t)width * height * 3);   //array
    if (image_s == NULL) {
      printf("malloc images_s error\n");
      return NULL;
    }
    fread(image_s, sizeof(unsigned char), (size_t)(long)width * height * 3, fp_s);
    fclose(fp_s);

    return image_s;
}


int get_raw(const char *fname_s, const char *fname_t) {
    FILE          *fp_t = NULL;    // target file handler
    unsigned int  x=0,y=0;             // for loop counter

    unsigned int file_size = 0;           // file size
    unsigned int rgb_raw_data_offset = 0; // RGB raw data offset
    unsigned char *image_t = NULL; // target image array
    unsigned char R = '\0', G = '\0', B = '\0';         // color of R, G, B
    unsigned int y_avg=0;            // average of y axle
    unsigned int y_t=0;              // target of y axle


    unsigned char header[54] = {
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


    unsigned char* image_s = read_bmp(fname_s, &rgb_raw_data_offset);

    image_t = (unsigned char *)malloc((size_t)width * height * 3);   //array
    if (image_t == NULL) {
      printf("malloc image_t error\n");
      return -1;
    }


    /* 以上為處理檔頭的前置作業*/
    y_avg = 0 + (height-1);
    int r = height;
    int c = width;
     //convert_to_image_array(image_s, height, width);
    for (int y=0; y < r; y++) {
        for(int x = 0; x < c; x++) {
            int idx = c * y + x;
            image_in[idx].R = *(image_s + 3 * idx + 2);
            image_in[idx].G = *(image_s + 3 * idx + 1);
            image_in[idx].B = *(image_s + 3 * idx + 0);
        }
    }

    depth_section();
    shift();


    for(y = 0; y != width; ++y) {
      for(x = 0; x != height; ++x) {
        *(image_t + 3 * (width * x + y) + 2) = image_shift[x][y].R;
        *(image_t + 3 * (width * x + y) + 1) = image_shift[x][y].G;
        *(image_t + 3 * (width * x + y) + 0) = image_shift[x][y].B;
      }
    }


    // write to new bmp
    fp_t = fopen(fname_t, "wb");
    if (fp_t == NULL) {
        printf("fopen fname_t error\n");
        return -1;
    }


     // file size
     file_size = width * height * 3 + rgb_raw_data_offset;
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


     // write header
     fwrite(header, sizeof(unsigned char), rgb_raw_data_offset, fp_t);
     // write image
     fwrite(image_t, sizeof(unsigned char), (size_t)(long)width * height * 3, fp_t);


     free(image_s);

     free(image_t);


     fclose(fp_t);


     return 0;
}


int get_depth(const char *fname_d)
{
     FILE          *fp_d = NULL;    // depth file handler
     unsigned char *image_d = NULL;   //depth image array
     unsigned int depth_raw_data_offset;
     int x=0;
     int y=0;
     int i=0;
     float dep_r=0, dep_g=0, dep_b=0;
     dep_max =-128;
     dep_min =127;

     fp_d = fopen(fname_d, "rb");
     if (fp_d == NULL) {
      printf("fopen fp_d error\n");
      perror("Error!");
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
     for(y = 0; y != height; ++y) {
         for(x = 0; x != width; ++x) {
             int idx = width * y + x;
             dep_r = *(image_d + 3 * idx + 2);
             dep_g = *(image_d + 3 * idx + 1);
             dep_b = *(image_d + 3 * idx + 0);
             image_in[idx].depth = (dep_r + dep_g + dep_b)/3;
             if(image_in[idx].depth > dep_max)
        dep_max = image_in[idx].depth;
         else if(image_in[idx].depth < dep_min)
                dep_min = image_in[idx].depth;
             //image_in[x][y].G = G;
             //image_in[x][y].B = B;
        }
    }
    fclose(fp_d);
    free(image_d);
    return 1;
}



void depth_section()
{
    int section;
    int divide;
    section = (dep_max - dep_min + 1) / 8;
    printf("%d\n", section);
    for(int i=0; i!=height; ++i)
    {
       for(int j=0; j!=width; ++j)
       {
          int idx = i * width + j;
          //--------------------shift-8-------------------------------------------
          if(image_in[idx].depth<= dep_min + section)  //dep_min + section
          {
            image_in[idx].shift_num = 1;  // (-128~-97)
            image_in[idx].priority = 1;   //closest 1st
          }
          else if(image_in[idx].depth<= dep_min + 2*section)
          {
            image_in[idx].shift_num = 2;  //
            image_in[idx].priority = 2;   //2nd
          }
          else if(image_in[idx].depth<= dep_min + 3*section)
          {
            image_in[idx].shift_num = 3;
            image_in[idx].priority = 3;   //3rd
          }
          else if(image_in[idx].depth<= dep_min + 4*section)
          {
            image_in[idx].shift_num = 4;
            image_in[idx].priority = 4;   //4rd
          }
          else if(image_in[idx].depth<= dep_min + 5*section)
          {
            image_in[idx].shift_num = 5;
            image_in[idx].priority = 5;   //5rd
          }
          else if(image_in[idx].depth<= dep_min + 6*section)
          {
            image_in[idx].shift_num = 6;  //
            image_in[idx].priority = 6;   //6rd
          }
          else if(image_in[idx].depth<= dep_min + 7*section)
          {
            image_in[idx].shift_num = 7;  // (64~95)
            image_in[idx].priority = 7;   //7rd
          }
          else if(image_in[idx].depth<= dep_min + 8*section)
          {
            image_in[idx].shift_num = 8;  // (96~127)
            image_in[idx].priority = 8;   //8th
          }
      }
     }
}


void shift()   // i = row, j = column
{
    int pointer = 0;
    float length = 0, counter = 0, son = 1, mom = 0;
    float temp_r_l, temp_r_r;
    float temp_g_l, temp_g_r;
    float temp_b_l, temp_b_r;
    int save_j=0, direction=0;
    for(int i=0 ; i<=height; i++)
        for(int j=0; j<=(width-1); j++)  //(image_size.width-1)
        {
            int idx=i*width+j;
            if(image_shift[i][j+image_in[idx].shift_num].on == 0)
            {
                image_shift[i][j+image_in[idx].shift_num].R = image_in[idx].R;
                image_shift[i][j+image_in[idx].shift_num].G = image_in[idx].G;
                image_shift[i][j+image_in[idx].shift_num].B = image_in[idx].B;
                image_shift[i][j+image_in[idx].shift_num].on = 1;
                image_shift[i][j+image_in[idx].shift_num].priority = image_in[idx].priority;
            }else{
                if(image_in[idx].priority > image_shift[i][j+image_in[idx].shift_num].priority)
                {
                    image_shift[i][j+image_in[idx].shift_num].R = image_in[idx].R;
                    image_shift[i][j+image_in[idx].shift_num].G = image_in[idx].G;
                    image_shift[i][j+image_in[idx].shift_num].B = image_in[idx].B;
                    image_shift[i][j+image_in[idx].shift_num].on = 1;
                    image_shift[i][j+image_in[idx].shift_num].priority = image_in[idx].priority;
                }
            }
        }// end of switch


    for(int m = 0; m<Width; m++)
        for(int n = 0; n<Height; n++)
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


  for(int i=0; i<Width; i++)
     for(int j=0; j<Height; j++)
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
           counter++;
        }
// ---------------------- intepolation filling -----------------------------------------
        save_j = j;
        while( image_shift[i][save_j].on == 0 )
        {
           mom = counter + 1;
           for(int k = counter; k > 0; k--)
           {
              float test_r = 0, test_b = 0, test_g = 0;
              test_r = ((float)son/(float)mom)*temp_r_r + ((float)(mom - son )/(float)mom)*temp_r_l;
              image_shift[i][save_j].R = test_r;
              test_g = ((float)son/(float)mom)*temp_g_r + ((float)(mom - son )/(float)mom)*temp_g_l;
              image_shift[i][save_j].G = test_g;
              test_b = ((float)son/(float)mom)*temp_b_r + ((float)(mom - son )/(float)mom)*temp_b_l;
              image_shift[i][save_j].B = test_b;
              son++;
              save_j++;
           }
        }
        counter = 0;
        son = 1;
        mom = 0;
     }
} // end of shift




int main()
{
int totalNumbers=1;
    char FileName[100], OutName[100], Dep[100];//百圖檔名用

    for(int Number=1; Number<=totalNumbers; Number++)
    {

        sprintf(Dep, "images/f%ddep.bmp", Number);
        sprintf(FileName, "images/f%d.bmp", Number);
        sprintf(OutName, "f%d_l2.bmp", Number);
        printf("\nDep=%s\n", Dep);
        printf("FileName=%s\n", FileName);
        printf("OutName=%s\n", OutName);

        get_depth(Dep);
        get_raw(FileName, OutName);
    }
    //system("pause");
    //free(image_in);
}
