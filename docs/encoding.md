# Image Encoding

# Helpers
int color_to_ch (color_t color)
 * Converts one color to its corresponding character
 * Input: 
  * color: color to convert
 * Ouput: 
  * character (in the form of an int) between 'a' and '{'
 
int color_distance(color_t color)
 * Finds the closest color in our defined set of colors to the input color
 * Input: 
  * color: untranslated color
 * Output: 
  * integer index corresponding to a color in our set of colors

color_t ch_to_color(int c)
 * Gets color corresponding to a character
 * Input: 
  * c: character (in the form of an int)
 * Output: 
  * color corresponding to the character in our color set
 
void compressed_file_to_file (compressed_file_t \*cf, char\* filename)
 * Writes the contents of a compressed_file_t struct to a text file
 * Input:
  * cf: a pointer to a compressed_file_t that has been initialized
 * Output: 
  * filename where to write the text contents to

color_t get_color(int i)
 * Returns color at that index in our color array, in order to access it outside image.c file
 * Input: 
  * i: the index of the color array you want to access
 * Output:
  * the color at that index

## Main functions

void convert_image(char\* input_image, char\* output_image, compressed_file_t \* file_ptr)
 * Converts input_image into an image with our limited color palette and saves it to the computer, and also saves the encoded text to a text file and a compressed_file_t struct
 * Input:
  * input_image: filename for the untranslated image
  * output_image: filename that this function should write to, to output the translated image
  * file_ptr: struct to save information that will also be saved in an actual txt file

void getImageFromFile(compressed_file_t \* file, char\* output_image)
 * Given a compressed file struct and a location to output the image, it will translate the text back into an image and save it to the device
 * Input:
  * file: an initialized compressed_file_t struct
  * output_image: the file name where the new image should be saved
