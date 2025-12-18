#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <wand/MagickWand.h>
#include "image.h"

int main(int argc,char **argv)
{
    if (argc != 3)
    {
      (void) fprintf(stdout,"Usage: %s image output-image\n",argv[0]);
      exit(0);
    }
    compressed_file_t * file = malloc(sizeof(compressed_file_t));
    convert_image(argv[1], argv[2], file);
    getImageFromFile(file, "images/test_convert_back.jpg");
    free(file->contents);
    free(file);
  return(0);
}