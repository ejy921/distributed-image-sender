#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <wand/MagickWand.h>
#include "image.h"

/* Adapted from example at https://bricomoral.com/ImageMagick-6.8.9-7/www/magick-wand.html */

/**
 * Convert input_image into an image with our limited color palette and save it to the computer, and also save the encoded text to a text file and a compressed_file_t struct
 * @param input_image filename for the untranslated image
 * @param output_image filename that this function should write to, to output the translated image
 * @param file_ptr struct to save information that will also be saved in an actual txt file
 */
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

  // My own separate variables for height and width that are ints instead 
  int h = MagickGetImageHeight(image_wand);
  int w = MagickGetImageWidth(image_wand);
  // worst case size of character array (if every adjacent pixel is a different color) 
  // would be two characters for each pixel plus the string terminator
  char * chpixels = malloc(h * w * 2 + 1); 
  char curr = 126; // starts at an unused character
  int curr_count = 0; // how many times in a row current character has been seen
  int curr_pos = 0; // position in chpixels string

  iterator=NewPixelIterator(image_wand);
  if (iterator == (PixelIterator *) NULL)
    ThrowWandException(image_wand);
  for (y=0; y < (long) MagickGetImageHeight(image_wand); y++) // y is also height
  {
    curr = 126; // reset for each row 
    curr_count = 0; //reset for each row
    // get a row of pixels
    pixels=PixelGetNextIteratorRow(iterator,&width);
    if (pixels == (PixelWand **) NULL)
      break;
    for (x=0; x < (long) width; x++)
    {
      PixelGetMagickColor(pixels[x],&pixel);
      // Our colors are smaller number than the ImageMagick colors, so divide by 256 first
      // Then convert to the closest of our colors
      color_t color = {pixel.red / 256, pixel.green / 256, pixel.blue / 256};
      int i = color_distance(color);
      color_t new_color = get_color(i);
      // Get the character representation of the translated color
      char new_char = (char) color_to_ch(new_color);
      // Previous pixel was also this color
      if (new_char == curr) {
        // Increase how many times we've seen this color
        curr_count++;
        if (x == w-1) { // if this is the last pixel in the row, we need to write the information to the file immediately 
          // If the count is more than nine, it needs to be reset
          if (curr_count > 9) {
            // set the next character to the character representation of 9
            chpixels[curr_pos] = '9';
            curr_pos++;
            chpixels[curr_pos] = curr; // start a new record of this character
            curr_pos++;
            chpixels[curr_pos] = '1'; // record a 1 for this character, this is the end of the row so we know it is 1
            curr_pos++;
          } else {
            chpixels[curr_pos] = (char)(curr_count + '0'); // record curent count before it is reset on the next row
            curr_pos++;
          }
        } else { // not the end of the row
          // If the count is more than nine, it needs to be reset
          if (curr_count > 9) {
            chpixels[curr_pos] = '9';
            curr_pos++;
            chpixels[curr_pos] = curr; // start a new record for this character
            curr_pos++;
            curr_count = 1; // we just started a new record, so we've seen this character once
          }
        }
      } else { // this pixel is a different color than the previous pixel
        if (curr_count > 0) { // don't record previous count if the count is still zero, but otherwise we need it
          chpixels[curr_pos] = (char)(curr_count + '0'); // record previous count for the previous character
          curr_pos++;
        }
        curr = new_char; 
        chpixels[curr_pos] = curr; // record new character
        curr_pos++;  
        curr_count = 1; // we've seen this character once
        if (x == w-1) { // if it's the last pixel of the row we must save the count immediately
          chpixels[curr_pos] = '1';
          curr_pos++;
        }
      }
      // Convert color values back to the ImageMagick range
      pixel.red= new_color.r * 256;
      pixel.green= new_color.g * 256;
      pixel.blue= new_color.b * 256;
      PixelSetMagickColor(pixels[x],&pixel);
    }
    // Sync changes
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
  MagickWandTerminus();
  // set end of the string to the string terminator
  chpixels[curr_pos] = '\0';
  // get length of the contents (different for each file and isn't connected to image resolution)
  int contents_length = strlen(chpixels);
  // set the struct fields
  file_ptr->w = w;
  file_ptr->h = h;
  file_ptr->contents_length = contents_length;
  // malloc space for this string and copy the local string into it
  file_ptr->contents=malloc(contents_length);
  strcpy(file_ptr->contents, chpixels);
  // save to a txt file (just to see)
  compressed_file_to_file(file_ptr, "testoutput.txt");
  // free local version of the string (it's most likely larger than the version we copied)
  free(chpixels);
}

/**
 * Given a compressed file struct and a location to output the image, it will translate the text back into an image and save it to the device
 * @param file an initialized compressed_file_t struct
 * @param output_image the file name where the new image should be saved
 */
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
  int file_pos = 0; // Keeps track of the next character to parse
  char curr; // saving the character for use in the loop
  MagickWandGenesis();
  image_wand=NewMagickWand();
  // Source for creating a blank image: https://imagemagick.org/api/magick-image.php 
  // create a new PixelWand for the background color and set it to white
  PixelWand * background_color = NewPixelWand();
  PixelSetColor(background_color, "white");
  // create a new white image of width w and height h, with dimensions sourced from the struct
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
    x = 0; // reset x
    // we can take care of multiple 'x' per loop, so a while loop instead of a for loop
    while (x < (long) width) {
      // get first character of the row
      curr = file->contents[file_pos];
      file_pos++;
      // second character is how many times the color corresponding to that character should appear
      char count_char = file->contents[file_pos];
      // convert back to integer
      int count = count_char - '0';
      file_pos++;
      // get to correct color for this character
      color_t new_color = ch_to_color((int)curr);
      // set pixels 'count' number of times
      for (int i = 0; i < count; i++) {
        PixelGetMagickColor(pixels[x + i],&pixel);
        // translate colors to the ImageMagick scale
        pixel.red = new_color.r * 256;
        pixel.green = new_color.g * 256;
        pixel.blue = new_color.b * 256;
        // set the correct pixel
        PixelSetMagickColor(pixels[x + i],&pixel);
      }
      // make sure x is updated
      x += count;
    }
    // sync changes
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