/*
Utility functions
*/
#include <stdint.h>

/// @brief convert a RGB color to one byte web-safe color
/// @param r 
/// @param g 
/// @param b 
/// @return offset of the resulting color in the 216 web-safe color table
uint8_t rgb2oneb(unsigned char r, unsigned char g, unsigned char b)
{
    // convert each element into the range of 0-5
    auto convert = [](unsigned char val) -> unsigned int
    {
        const unsigned int denom = 256 / 6;
        unsigned int t = val / denom;
        return t;
    };
    return convert(r) * 6 * 6 + convert(g) * 6 + convert(b);
}