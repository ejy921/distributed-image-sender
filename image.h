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

int color_to_ch (color_t color);
int color_distance(color_t color);
color_t ch_to_color(int c);
void fill_file (char* filename, color_t pixels[], int w, int h);
void compressed_file_to_file (compressed_file_t *cf, char* filename);
color_t get_color(int i);