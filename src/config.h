#ifndef CONFIG_H
#define CONFIG_H

#define COLOR_FORMAT WL_SHM_FORMAT_ARGB8888
#define COLOR_DEPTH  4

#define COLOR_BG     0xFF282828
#define COLOR_FG     0xFFEBDBB2
#define COMMAND_BG   0xFF504945
#define BORDER_COLOR 0xFF3C3836

#define WIDTH          800
#define HEIGHT         400
#define MARGIN         0
#define COMMAND_HEIGHT 32 // 32
#define PADDING        15
#define BORDER_WIDTH   2

#define INITIAL_COMMAND ":"

#define TITLE ":woof"

#define DOUBLEKEYLIMIT 100 // in ms

#define FONT_NAME  "Sarasa Term J Nerd Font"
#define FONT_SCALE 18
#endif
