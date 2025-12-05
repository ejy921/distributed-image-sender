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
  // return int of character value a-{
  return color_i + OFFSET;
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

int main(int argc, char** argv) {
  //For testing

  color_t testcolor = {3, 64, 129};
  int result = color_to_ch(testcolor);
  printf("Converted to: %c\n", (char) result);
  color_t convertback = ch_to_color(result);
  printf("Converted back to: {%d, %d, %d}\n", convertback.r, convertback.g, convertback.b);
  color_t test_pixels[] = {{0, 0, 0}, {35, 203, 127}, {200, 150, 254}, {0, 0, 0}, {255, 127, 255}, {255, 255, 255}};
  fill_file("testfile.txt", test_pixels, 2, 3);

}

