// Firefly
//
// This is a firefly-themed pattern for an 8x8 RGB LED array connected to an Arduino
// It currently requires Adafruit's Neopixel library
//
//
//

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
const uint32_t NumPixels = 64;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NumPixels, PIN, NEO_GRB + NEO_KHZ800);


// Color palette
// c9ff2f
// b4e52a
// a0cc25

uint32_t nightColor = strip.Color(0, 0, 8);
uint32_t fireflyColor = strip.Color(0x50, 0x66, 0x12);

typedef struct Pixel
{
  uint32_t color;
};
uint32_t backgroundTimer = 0;
uint32_t BackgroundDelay = 100;
uint8_t BackgroundSame = 0;
uint8_t BackgroundConverge = 10;
uint8_t BackgroundDiverge = 100;

const uint8_t MaxFireflies = 3;
typedef struct Firefly
{
  uint8_t pixel;
  uint32_t currentColor;
  uint32_t targetColor;
  uint32_t tick;
  uint32_t duration;
};
uint32_t nextFireflyTimer = 0;
uint32_t FireflyWaitMin = 600;
uint32_t FireflyWaitMax = 1500;
uint32_t FireflyDurationMin = 400;
uint32_t FireflyDurationMax = 1200;

Pixel pixels[NumPixels];
Firefly firefly[MaxFireflies];

void setup() {
  strip.begin();
  strip.setBrightness(128);

  fill(nightColor);
  strip.show();

  for (uint8_t i=0; i < NumPixels; i++) {
    pixels[i].color = nightColor;
  }
  for (uint8_t i=0; i < MaxFireflies; i++) {
    clearFirefly(firefly[i]);
  }
  startFirefly(firefly[0]);
  nextFireflyTimer = random(FireflyWaitMin, FireflyWaitMax);
}

void loop() {

  backgroundTimer++;
  if (backgroundTimer >= BackgroundDelay) {
    jitterBackground();
    backgroundTimer = 0;
  }
  
  for (uint8_t i=0; i < MaxFireflies; i++) {
    updateFirefly(firefly[i]);
    if (firefly[i].pixel != 255) {
      strip.setPixelColor(firefly[i].pixel, firefly[i].currentColor);
    }
  }

  nextFireflyTimer--;
  if (!nextFireflyTimer == 0) {
    uint8_t nextFirefly = -1;
    for (uint8_t i=0; i < MaxFireflies; i++) {
      if (firefly[i].pixel == 255) {
        nextFirefly = i;
        break;
      }
    }
    if (nextFirefly != -1) {
      startFirefly(firefly[nextFirefly]);
      nextFireflyTimer = random(FireflyWaitMin, FireflyWaitMax);
    }
    else {
      nextFireflyTimer = 1; // Try again next iteration
    }
  }
  
  strip.show();
  delay(1);
}

void jitterBackground() {
  uint8_t bNight = B(nightColor);
  for (uint8_t i=0; i < NumPixels; i++) {
    uint8_t r = random(100);
    if (r < BackgroundSame) {
      // No change
    }
    else if (r < BackgroundConverge) {
      uint8_t bCurrent = B(pixels[i].color);
      if (bCurrent < bNight)
        bCurrent++;
      else if (bCurrent > bNight)
        bCurrent--;
      pixels[i].color = strip.Color(0, 0, bCurrent);
    }
    else if (r < BackgroundDiverge) {
      uint8_t bCurrent = B(pixels[i].color);
      if (bCurrent < bNight && bCurrent > 0)
        bCurrent--;
      else if (bCurrent > bNight && bCurrent < 255)
        bCurrent++;
      pixels[i].color = strip.Color(0, 0, bCurrent);
    }
    strip.setPixelColor(i, pixels[i].color);
  }
}

void updateFirefly(Firefly& ff) {

  ff.tick++;
  float d = ff.tick / (float)(ff.duration / 2);

  if (ff.targetColor == fireflyColor) {
    uint8_t r = fade(R(nightColor), R(fireflyColor), d);
    uint8_t g = fade(G(nightColor), G(fireflyColor), d);
    uint8_t b = fade(B(nightColor), B(fireflyColor), d);
    ff.currentColor = strip.Color(r, g, b);
    
    if (ff.tick == ff.duration / 2) {
      ff.targetColor = nightColor;
      ff.tick = 0;
    }
  }
  else {
    uint8_t r = fade(R(fireflyColor), R(nightColor), d);
    uint8_t g = fade(G(fireflyColor), G(nightColor), d);
    uint8_t b = fade(B(fireflyColor), B(nightColor), d);
    ff.currentColor = strip.Color(r, g, b);
    
    if (ff.tick == ff.duration / 2) {
      ff.pixel = 255;
    }
  }
}

void clearFirefly(Firefly& ff) {
  ff.pixel = 255;
  ff.currentColor = nightColor;
  ff.targetColor = fireflyColor;
  ff.tick = 0;
  ff.duration = 0;
}

void startFirefly(Firefly& ff) {
  clearFirefly(ff);
  ff.pixel = random(strip.numPixels());
  ff.duration = (random(FireflyDurationMin, FireflyDurationMax) / 2) * 2;
}

void setRandomPixel(uint32_t colBase, uint32_t colRand, uint8_t threshold) {
  uint8_t pixel = random(strip.numPixels());
  if (random(100) < threshold) {
    strip.setPixelColor(pixel, colRand);
  }
  else {
    strip.setPixelColor(pixel, colBase);
  }
  strip.show();
}

void fill(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}

uint8_t fade(uint8_t c1, uint8_t c2, float d) {
  return (uint8_t)(((int32_t)c2 - c1) * d + c1);
}

// Per Adafruit Neopixel docs:
// Packed format is always RGB, regardless of LED strand color order.
uint8_t R(uint32_t col) { return (col >> 16) & 0xff; }
uint8_t G(uint32_t col) { return (col >> 8) & 0xff; }
uint8_t B(uint32_t col) { return col & 0xff; }

