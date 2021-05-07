#ifndef LED_SETTINGS_H
#define LED_SETTINGS_H

#include <stdint.h>
#include <FastLED.h>

#ifndef LED_PIN
#define LED_PIN 1 // PIN LED LINE
#endif
#ifndef COLOR_ORDER
#define COLOR_ORDER GRB
#endif
#ifndef CHIPSET
#define CHIPSET WS2811
#endif
#ifndef BRIGHTNESS
#define BRIGHTNESS 3 //64
#endif

// Params for width and height
#ifndef MATRIX_WIDTH
#define MATRIX_WIDTH 16
#endif
#ifndef MATRIX_HEIGHT
#define MATRIX_HEIGHT 16
#endif
#ifndef NUM_LEDS
#define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
#endif

// Add "safety pixel". See more info to example XYMatrix.ino
CRGB leds_plus_safety_pixel[NUM_LEDS + 1];
CRGB *const leds(leds_plus_safety_pixel + 1);

// Param for different pixel layouts
const bool kMatrixSerpentineLayout = true;
const bool kMatrixVertical = false;

// Set 'kMatrixSerpentineLayout' to false if your pixels are
// laid out all running the same way, like this:
//
//     0 >  1 >  2 >  3 >  4
//                         |
//     .----<----<----<----'
//     |
//     5 >  6 >  7 >  8 >  9
//                         |
//     .----<----<----<----'
//     |
//    10 > 11 > 12 > 13 > 14
//                         |
//     .----<----<----<----'
//     |
//    15 > 16 > 17 > 18 > 19
//
// Set 'kMatrixSerpentineLayout' to true if your pixels are
// laid out back-and-forth, like this:
//
//     0 >  1 >  2 >  3 >  4
//                         |
//                         |
//     9 <  8 <  7 <  6 <  5
//     |
//     |
//    10 > 11 > 12 > 13 > 14
//                        |
//                        |
//    19 < 18 < 17 < 16 < 15
//
// Bonus vocabulary word: anything that goes one way
// in one row, and then backwards in the next row, and so on
// is call "boustrophedon", meaning "as the ox plows."

// This function will return the right 'led index number' for
// a given set of X and Y coordinates on your matrix.
// IT DOES NOT CHECK THE COORDINATE BOUNDARIES.
// That's up to you.  Don't pass it bogus values.
//
// Use the "XY" function like this:
//
//    for( uint8_t x = 0; x < kMatrixWidth; x++) {
//      for( uint8_t y = 0; y < kMatrixHeight; y++) {
//
//        // Here's the x, y to 'led index' in action:
//        leds[ XY( x, y) ] = CHSV( random8(), 255, 255);
//
//      }
//    }

// Helper functions for an two-dimensional XY matrix of pixels.
// Simple 2-D demo code is included as well.
//
//     XY(x,y) takes x and y coordinates and returns an LED index number,
//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;
//             No error checking is performed on the ranges of x and y.
//
//     XYsafe(x,y) takes x and y coordinates and returns an LED index number,
//             for use like this:  leds[ XY(x,y) ] == CRGB::Red;
//             Error checking IS performed on the ranges of x and y, and an
//             index of "-1" is returned.  Special instructions below
//             explain how to use this without having to do your own error
//             checking every time you use this function.
//             This is a slightly more advanced technique, and
//             it REQUIRES SPECIAL ADDITIONAL setup, described below.

uint16_t XY(uint8_t x, uint8_t y)
{
  uint16_t i;

  if (kMatrixSerpentineLayout == false)
  {
    if (kMatrixVertical == false)
    {
      i = (y * MATRIX_WIDTH) + x;
    }
    else
    {
      i = MATRIX_HEIGHT * (MATRIX_WIDTH - (x + 1)) + y;
    }
  }

  if (kMatrixSerpentineLayout == true)
  {
    if (kMatrixVertical == false)
    {
      if (y & 0x01)
      {
        // Odd rows run backwards
        uint8_t reverseX = (MATRIX_WIDTH - 1) - x;
        i = (y * MATRIX_WIDTH) + reverseX;
      }
      else
      {
        // Even rows run forwards
        i = (y * MATRIX_WIDTH) + x;
      }
    }
    else
    { // vertical positioning
      if (x & 0x01)
      {
        i = MATRIX_HEIGHT * (MATRIX_WIDTH - (x + 1)) + y;
      }
      else
      {
        i = MATRIX_HEIGHT * (MATRIX_WIDTH - x) - (y + 1);
      }
    }
  }

  return i;
}

// USE ONLY WITH THE "SAFETY PIXEL" LEDS
uint16_t XYsafe(uint8_t x, uint8_t y)
{
  if (x >= MATRIX_WIDTH)
    return -1;
  if (y >= MATRIX_HEIGHT)
    return -1;
  return XY(x, y);
}

uint32_t colorIndex(uint16_t i)
{
  return (((uint32_t)leds[i].r << 16) | ((long)leds[i].g << 8) | (long)leds[i].b);
}

uint32_t colorXY(uint8_t x, uint8_t y)
{
  uint16_t i = XYsafe(x, y);
  return colorIndex(i);
}

#endif