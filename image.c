#include "image.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
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

int main(int argc, char** argv) {
  //For testing

  color_t testcolor = {3, 64, 129};
  int result = color_to_ch(testcolor);
  printf("Converted to: %c\n", (char) result);
  color_t convertback = ch_to_color(result);
  printf("Converted back to: {%d, %d, %d}\n", convertback.r, convertback.g, convertback.b);

}

