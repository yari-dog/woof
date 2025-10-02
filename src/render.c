#include <stdint.h>
void
render (uint32_t *buffer, uint32_t size, uint32_t width, uint32_t height)
{
    // print checkerboard
    uint32_t bg    = 0xFF282828;
    uint32_t color = 0xFFEBDBB2;
    for (int i = 0; i < height; ++i)
        {
            for (int j = 0; j < width; ++j)
                {
                    if ((i + j / 8 * 8) % 16 < 8)
                        buffer[i * width + j] = color;
                    else
                        buffer[i * width + j] = bg;
                }
        }
}
