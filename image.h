typedef struct color {
  int r;
  int g;
  int b;
} color_t;

typedef struct __attribute__((packed)) compressed_file {
  int h;
  int w;
  int contents_length;
  char * contents;
} compressed_file_t;

/**
 * Convert one color to its corresponding character
 * @param color color to convert
 * @returns character (in the form of an int) between 'a' and '{'
 */
int color_to_ch (color_t color);

/**
 * Find the closest color in our defined set of colors to the input color
 * @param color untranslated color
 * @returns integer index corresponding to a color in our set of colors
 */
int color_distance(color_t color);

/**
 * Get color corresponding to a character
 * @param c character (in the form of an int)
 * @returns color corresponding to the character in our color set
 */
color_t ch_to_color(int c);

/**
 * Write the contents of a compressed_file_t struct to a text file
 * @param cf a pointer to a compressed_file_t that has been initialized
 * @param filename where to write the text contents to
 */
void compressed_file_to_file (compressed_file_t *cf, char* filename);

/**
 * Return color at that index in our color array, in order to access it outside this file
 * @param i the index of the color array you want to access
 * @returns color at that index
 */
color_t get_color(int i);

/**
 * Convert input_image into an image with our limited color palette and save it to the computer, and also save the encoded text to a text file and a compressed_file_t struct
 * @param input_image filename for the untranslated image
 * @param output_image filename that this function should write to, to output the translated image
 * @param file_ptr struct to save information that will also be saved in an actual txt file
 */
void convert_image(char* input_image, char* output_image, compressed_file_t * file_ptr);

/**
 * Given a compressed file struct and a location to output the image, it will translate the text back into an image and save it to the device
 * @param file an initialized compressed_file_t struct
 * @param output_image the file name where the new image should be saved
 */
void getImageFromFile(compressed_file_t * file, char* output_image);