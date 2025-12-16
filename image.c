#include "image.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define NUM_COLORS 27
#define OFFSET 97

color_t colors[] = {{0, 0, 0},
                    {0, 0, 127},
                    {0, 0, 255},
                    {0, 127, 0},
                    {0, 127, 127},
                    {0, 127, 255},
                    {0, 255, 0},
                    {0, 255, 127},
                    {0, 255, 255},
                    {127, 127, 127},
                    {127, 127, 0},
                    {127, 127, 255},
                    {127, 0, 0},
                    {127, 0, 127},
                    {127, 0, 255},
                    {127, 255, 0},
                    {127, 255, 127},
                    {127, 255, 255},
                    {255, 255, 255},
                    {255, 255, 0},
                    {255, 255, 127},
                    {255, 0, 0},
                    {255, 0, 127},
                    {255, 0, 255},
                    {255, 127, 0},
                    {255, 127, 127},
                    {255, 127, 255}};

int color_to_ch(color_t color) {
  int color_i = color_distance(color);
  // return int of character value a-{
  return color_i + OFFSET;
}

int color_distance(color_t color) {
  //get three values
  int r = color.r;
  int g = color.g;
  int b = color.b;
  //distance formula with default colors
  double min_distance = 255.0;
  int color_i = -1;
  for (int i = 0; i < NUM_COLORS; i++) {
    int r_ = colors[i].r;
    int g_ = colors[i].g;
    int b_ = colors[i].b;

    double x = r_ - r;
    double y = g_ - g;
    double z = b_ - b;

    double distance = fabs(sqrt(pow(x, 2.0) + pow(y, 2.0) + pow(z, 2.0)));
    if (distance < min_distance) {
      min_distance = distance;
      color_i = i;
    }
  }
  
  return color_i;
}

color_t ch_to_color(int c) {
  // return color at the same position as the character
  return (colors[c - OFFSET]);

}

void fill_file (char* filename, color_t pixels[], int w, int h) {
  FILE* file = fopen(filename, "w");
  for (int i = 0; i < h; i++) {
    for (int j = 0; j < w; j++) {
      char c = (char) color_to_ch(pixels[i * w + j]);
      fwrite(&c, 1, 1, file);
    }
  }
  fclose(file);
}

void compressed_file_to_file (compressed_file_t *cf, char* filename){
FILE* file = fopen(filename, "w");
fprintf(file,"W:%d\nH:%d\nL:%d\n%s", cf->w, cf->h, cf->contents_length, cf->contents);
}

// void file_to_compressed_file (char* filename, compressed_file_t * compressed_file) {
//   FILE* file = fopen(filename, "r");
//   int w;
//   int h;
//   int l;
//   char * contents;
//   fscanf(file, "W:%d\nH:%d\nL:%d\n%s", &w, &h, &l, contents);
// }

color_t get_color(int i) {
  return colors[i];
}


