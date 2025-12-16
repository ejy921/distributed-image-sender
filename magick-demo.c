#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <wand/MagickWand.h>
#include "image.h"


void convert_image(char* input_image, char* output_image, compressed_file_t * file_ptr) {
  #define ThrowWandException(wand) \
{ \
  char \
    *description; \
 \
  ExceptionType \
    severity; \
 \
  description=MagickGetException(wand,&severity); \
  (void) fprintf(stderr,"%s %s %lu %s\n",GetMagickModule(),description); \
  description=(char *) MagickRelinquishMemory(description); \
  exit(-1); \
}

  long
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    pixel;

  MagickWand
    *contrast_wand,
    *image_wand;

  PixelIterator
    *contrast_iterator,
    *iterator;

  PixelWand
    **contrast_pixels,
    **pixels;

  register long
    x;

  unsigned long
    width;


  /*
    Read an image.
  */
  MagickWandGenesis();
  image_wand=NewMagickWand();
  status=MagickReadImage(image_wand,input_image);
  if (status == MagickFalse)
    ThrowWandException(image_wand);
  //contrast_wand=CloneMagickWand(image_wand);
  int h = MagickGetImageHeight(image_wand);
  int w = MagickGetImageWidth(image_wand);
  char * chpixels = malloc(h * w * 2 + 1); // worst case size of character array (if every adjacent pixel is a different color)
  char curr = 126; // starts at an unused character
  int curr_count = 0;
  int curr_pos = 0;

  iterator=NewPixelIterator(image_wand);
  //contrast_iterator=NewPixelIterator(contrast_wand);
  // if ((iterator == (PixelIterator *) NULL) ||
  //     (contrast_iterator == (PixelIterator *) NULL))
  if (iterator == (PixelIterator *) NULL)
    ThrowWandException(image_wand);
  for (y=0; y < (long) MagickGetImageHeight(image_wand); y++)
  {
    curr = 126;
    curr_count = 0;
    pixels=PixelGetNextIteratorRow(iterator,&width);
    // contrast_pixels=PixelGetNextIteratorRow(contrast_iterator,&width);
    if (pixels == (PixelWand **) NULL)
      break;
    for (x=0; x < (long) width; x++)
    {
      PixelGetMagickColor(pixels[x],&pixel);
      //printf("%f, %f, %f\n", pixel.red, pixel.green, pixel.blue);
      color_t color = {pixel.red / 256, pixel.green / 256, pixel.blue / 256};
      int i = color_distance(color);
      color_t new_color = get_color(i);
      char new_char = (char) color_to_ch(new_color);
      if (new_char == curr) {
        curr_count++;
        if (x == w-1) {
          if (curr_count > 9) {
            chpixels[curr_pos] = (char)(9 + '0');
            curr_pos++;
            chpixels[curr_pos] = curr;
            curr_pos++;
            chpixels[curr_pos] = (char)(1 + '0');
            curr_pos++;
          } else {
            chpixels[curr_pos] = (char)(curr_count + '0');
            curr_pos++;
          }
        } else {
          if (curr_count > 9) {
            chpixels[curr_pos] = (char)(9 + '0');
            curr_pos++;
            chpixels[curr_pos] = curr;
            curr_pos++;
            curr_count = 1;
          }
        }
      } else {
        if (curr_count > 0) {
          chpixels[curr_pos] = (char)(curr_count + '0');
          curr_pos++;
        }
        curr = new_char;
        chpixels[curr_pos] = curr;
        curr_pos++;  
        curr_count = 1;
        if (x == w-1) {
          chpixels[curr_pos] = (char)(curr_count + '0');
          curr_pos++;
        }
      }
      pixel.red= new_color.r * 256;
      pixel.green= new_color.g * 256;
      pixel.blue= new_color.b * 256;
      //pixel.index=SigmoidalContrast(pixel.index);
      PixelSetMagickColor(pixels[x],&pixel);
    }
    (void) PixelSyncIterator(iterator);
  }
  if (y < (long) MagickGetImageHeight(image_wand))
    ThrowWandException(image_wand);
  //contrast_iterator=DestroyPixelIterator(contrast_iterator);
  iterator=DestroyPixelIterator(iterator);
  //image_wand=DestroyMagickWand(image_wand);
  /*
    Write the image then destroy it.
  */
  status=MagickWriteImages(image_wand,output_image,MagickTrue);
  if (status == MagickFalse)
    ThrowWandException(image_wand);
  // contrast_wand=DestroyMagickWand(contrast_wand);
  image_wand=DestroyMagickWand(image_wand);
  MagickWandTerminus();
  chpixels[curr_pos] = '\0';
  int contents_length = strlen(chpixels);
  file_ptr->w = w;
  file_ptr->h = h;
  file_ptr->contents_length = contents_length;
  file_ptr->contents=malloc(contents_length);
  strcpy(file_ptr->contents, chpixels);
  //printf("%s\n", chpixels);
  compressed_file_to_file(file_ptr, "testoutput.txt");
  free(chpixels);
}

void getImageFromFile(compressed_file_t * file, char* output_image) {
  #define ThrowWandException(wand) \
{ \
  char \
    *description; \
 \
  ExceptionType \
    severity; \
 \
  description=MagickGetException(wand,&severity); \
  (void) fprintf(stderr,"%s %s %lu %s\n",GetMagickModule(),description); \
  description=(char *) MagickRelinquishMemory(description); \
  exit(-1); \
}

  long
    y;

  MagickBooleanType
    status;

  MagickPixelPacket
    pixel;

  MagickWand
    *image_wand;

  PixelIterator
    *iterator;

  PixelWand
    **pixels;

  register long
    x;

  unsigned long
    width;


  /*
    Create a blank image
  */
  int file_pos = 0;
  char curr;
  MagickWandGenesis();
  image_wand=NewMagickWand();
  PixelWand * background_color = NewPixelWand();
  PixelSetColor(background_color, "white");
  status=MagickNewImage(image_wand, (size_t)file->w, (size_t)file->h, background_color);
  if (status == MagickFalse) {
    ThrowWandException(image_wand);
  }
  iterator=NewPixelIterator(image_wand);
  if (iterator == (PixelIterator *) NULL) {
    ThrowWandException(image_wand);
  }
  for (y=0; y < (long) MagickGetImageHeight(image_wand); y++) {
    pixels=PixelGetNextIteratorRow(iterator,&width);
    if (pixels == (PixelWand **) NULL) {
      break;
    }
    x = 0;
    while (x < (long) width) {
      curr = file->contents[file_pos];
      file_pos++;
      char count_char = file->contents[file_pos];
      int count = count_char - '0';
      file_pos++;
      color_t new_color = ch_to_color((int)curr);
      for (int i = 0; i < count; i++) {
        PixelGetMagickColor(pixels[x + i],&pixel);
        pixel.red = new_color.r * 256;
        pixel.green = new_color.g * 256;
        pixel.blue = new_color.b * 256;
        PixelSetMagickColor(pixels[x + i],&pixel);
      }
      x += count;
    }
    (void) PixelSyncIterator(iterator);
  }
  if (y < (long) MagickGetImageHeight(image_wand))
    ThrowWandException(image_wand);
  iterator=DestroyPixelIterator(iterator);
  /*
    Write the image then destroy it.
  */
  status=MagickWriteImages(image_wand,output_image,MagickTrue);
  if (status == MagickFalse)
    ThrowWandException(image_wand);
  image_wand=DestroyMagickWand(image_wand);
  background_color=DestroyPixelWand(background_color);

  MagickWandTerminus();
}


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