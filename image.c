#include "image.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define NUM_COLORS 27
#define OFFSET 97

// Every combination of 0, 127, and 255 for RBG
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

/**
 * Convert one color to its corresponding character
 * @param color color to convert
 * @returns character (in the form of an int) between 'a' and '{'
 */
int color_to_ch(color_t color) {
  // find closest color
  int color_i = color_distance(color);
  // return int of character value a-{
  return color_i + OFFSET;
}

/**
 * Find the closest color in our defined set of colors to the input color
 * @param color untranslated color
 * @returns integer index corresponding to a color in our set of colors
 */
int color_distance(color_t color) {
  //get three values
  int r = color.r;
  int g = color.g;
  int b = color.b;
  //distance formula with default colors
  double min_distance = 255.0;
  int color_i = -1;
  // compare to each possibility
  for (int i = 0; i < NUM_COLORS; i++) {
    int r_ = colors[i].r;
    int g_ = colors[i].g;
    int b_ = colors[i].b;

    double x = r_ - r;
    double y = g_ - g;
    double z = b_ - b;

    double distance = fabs(sqrt(pow(x, 2.0) + pow(y, 2.0) + pow(z, 2.0)));
    // if it's closer than the current min_distance, update 
    if (distance < min_distance) {
      min_distance = distance;
      color_i = i;
    }
  }
  
  return color_i;
}

/**
 * Get color corresponding to a character
 * @param c character (in the form of an int)
 * @returns color corresponding to the character in our color set
 */
color_t ch_to_color(int c) {
  // return color at the same position as the character
  return (colors[c - OFFSET]);

}

/**
 * Write the contents of a compressed_file_t struct to a text file
 * @param cf a pointer to a compressed_file_t that has been initialized
 * @param filename where to write the text contents to
 */
void compressed_file_to_file (compressed_file_t *cf, char* filename){
FILE* file = fopen(filename, "w");
// Header information: W=width of the image H=height of the image L=length of the "contents" string
fprintf(file,"W:%d\nH:%d\nL:%d\n%s", cf->w, cf->h, cf->contents_length, cf->contents);
}

/**
 * Return color at that index in our color array, in order to access it outside this file
 * @param i the index of the color array you want to access
 * @returns color at that index
 */
color_t get_color(int i) {
  return colors[i];
}


