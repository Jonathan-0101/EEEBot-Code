#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define leftPixelStripPin 17  // Arduino pin that connects to left NeoPixel strip
#define rightPixelStripPin 25 // Arduino pin that connects to ritht NeoPixel strip
#define NUM_PIXELS 8          // The number of LEDs (pixels) on NeoPixel

Adafruit_NeoPixel leftPixelStrip(NUM_PIXELS, leftPixelStripPin, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel rightPixelStrip(NUM_PIXELS, rightPixelStripPin, NEO_GRB + NEO_KHZ800);

void setup()
{
  leftPixelStrip.begin();
  rightPixelStrip.begin();
}

void loop()
{
  leftPixelStrip.clear();
  rightPixelStrip.clear();

  // turn pixels to green one by one with delay between each pixel
  for (int pixel = 0; pixel < NUM_PIXELS; pixel++)
  {                                                                        // for each pixel
    leftPixelStrip.setPixelColor(pixel, leftPixelStrip.Color(255, 50, 0)); // it only takes effect if pixels.show() is called
    leftPixelStrip.show();                                                 // send the updated pixel colors to the NeoPixel hardware.

    delay(150); // pause between each pixel
  }
  leftPixelStrip.clear();
  leftPixelStrip.show();
  for (int pixel = 0; pixel < NUM_PIXELS; pixel++)
  {                                                                          // for each pixel
    rightPixelStrip.setPixelColor(pixel, rightPixelStrip.Color(255, 50, 0)); // it only takes effect if pixels.show() is called
    rightPixelStrip.show();                                                  // send the updated pixel colors to the NeoPixel hardware.

    delay(150); // pause between each pixel
  }
  rightPixelStrip.clear();
  rightPixelStrip.show();
}