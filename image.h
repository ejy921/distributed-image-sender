typedef struct color {
  int r;
  int g;
  int b;
} color_t;

int color_to_ch (color_t color);
color_t ch_to_color(int c);