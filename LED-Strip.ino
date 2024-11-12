#include <FastLED.h>

#define LED_PIN 8
#define LED_COUNT 300

#define TOP_R 60
#define DESKLAMP_R 130
#define DESKLAMP_L 150
#define DESKLAMP2_R 75
#define DESKLAMP2_L 100
#define TOP_L 173
#define BOTTOM_L 228

#define Y_PIN A1
#define X_PIN A2
#define BUTTON 9
#define PIN_DIF 430

// STATIC
CRGB leds[LED_COUNT];

// VARIABLES
int r;
int g;
int b;

int chunkSize;
int chunkOffCount;

int shouldAnimateTop = 0;
int animationChunkSize = 1;
int animationDuration = 0;

// CONTROL
int didChange = 1;
int isLampOn = 0;

#define COLOR_WHITE 0
#define COLOR_COLD 1
#define COLOR_WARM 2
#define COLOR_RED 3
int color = COLOR_RED;

int shouldIgnoreButton = 0;
int shouldIgnorePin = 0;

int isPinLow(int x) {
  return (x < 500 - PIN_DIF) ? 1 : 0;
}
int isPinHigh(int x) {
  return (x > 500 + PIN_DIF) ? 1 : 0;
}

// SETUP
void setup() {
  Serial.begin(9600);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_COUNT);

  pinMode(BUTTON, INPUT_PULLUP);
}

// UTIL
void setLEDSection(int first, int last) {
  int i;

  int j = 0;
  int isOn = 0;

  for (i = first; i < last + 1; i++) {
    if (chunkOffCount == 0) {
      isOn = 1;
    } else if (j == chunkOffCount) {
      isOn = 1;
    } else if (j == chunkSize) {
      isOn = 0;
      j = 0;
    }
    j++;

    if (isOn == 1) {
      leds[i] = CRGB(r, g, b);
    } else {
      leds[i] = CRGB(0, 0, 0);
    }
    if (animationDuration == 0) {
      continue;
    }

    if (i % animationChunkSize == 0) {
      delay(animationDuration);
      FastLED.show();
    }
  }
  FastLED.show();
}

void allOff() {
  r = 0;
  g = 0;
  b = 0;

  setLEDSection(0, LED_COUNT);
}

void setTopAnimation() {
  if (shouldAnimateTop == 0) return;
  animationDuration = 2;
  animationChunkSize = 3;
}

void changeColor(int newColor) {
  if (color == newColor || shouldIgnorePin == 1) {
    return;
  }

  color = newColor;
  didChange = 1;
  shouldIgnorePin = 1;
}
// COLORS
void setWhite() {
  r = 255;
  g = 255;
  b = 255;
}

void setWarmWhite() {
  r = 255;
  g = 80;
  b = 10;
}

void setCold() {
  r = 150;
  g = 250;
  b = 255;
}

void setWarm() {
  r = 255;
  g = 50;
  b = 0;
}

void setRed() {
  r = 255;
  g = 0;
  b = 0;
}

void setDarkRed() {
  r = 1;
  g = 0;
  b = 0;
}

void changeBrightness(float factor) {
  r *= factor;
  g *= factor;
  b *= factor;
}

// CHUNKS
void resetChunks() {
  chunkSize = 1;
  chunkOffCount = 0;
}

void setChunks(int size, int off) {
  chunkSize = size;
  chunkOffCount = off;
}

// RANGES
void applyAll() {
  setLEDSection(0, LED_COUNT);
}

void applyDesklamp() {
  resetChunks();
  setLEDSection(DESKLAMP2_R, DESKLAMP2_L);
  setLEDSection(DESKLAMP_R, DESKLAMP_L);
}

void applyTop() {
  setLEDSection(TOP_R, TOP_L);
}

void applyBottom() {
  setLEDSection(BOTTOM_L, LED_COUNT);
}

void applyLeft() {
  setLEDSection(TOP_L, BOTTOM_L);
}

void applyRight() {
  setLEDSection(0, TOP_R);
}

void applyAllLowerGroups() {
  applyLeft();
  applyRight();
  applyBottom();
}

// PRESETS
void whiteDesklamp() {
  setWhite();
  applyDesklamp();
}

void warmDesklamp() {
  setWarmWhite();
  applyDesklamp();
}

void coldDesklamp() {
  setCold();
  applyDesklamp();
}

void redDesklamp() {
  setRed();
  applyDesklamp();
}

void warm() {
  setWarm();
  changeBrightness(0.3);
  setChunks(2, 1);
  applyAll();
}

void warmWithDesklamp() {
  setWarm();
  changeBrightness(0.2);
  setChunks(2, 1);
  applyAll();

  setTopAnimation();
  warmDesklamp();
}

void cold() {
  setCold();
  changeBrightness(0.3);
  setChunks(2, 1);
  applyAll();
  resetChunks();
}

void coldWithDesklamp() {
  setCold();
  changeBrightness(0.1);
  setChunks(2, 1);
  applyAll();
  resetChunks();

  setTopAnimation();
  coldDesklamp();
}

void lowRed() {
  setDarkRed();
  setChunks(20, 19);
  applyTop();
  applyRight();
  applyBottom();
  resetChunks();
}

void redWithDesklamp() {
  setDarkRed();
  setChunks(3, 2);
  applyRight();
  resetChunks();

  setRed();
  changeBrightness(0.1);
  applyBottom();

  setTopAnimation();
  redDesklamp();
}

// LOOP
void loop() {
  if (digitalRead(BUTTON) == 0) {
    if (shouldIgnoreButton == 0) {
      isLampOn = isLampOn == 0 ? 1 : 0;
      shouldIgnoreButton = 1;
      shouldAnimateTop = 1;
      didChange = 1;
    }
  } else {
    shouldIgnoreButton = 0;
  }

  int x = analogRead(X_PIN);
  int y = analogRead(Y_PIN);

  if (isPinLow(x) == 1) {
    changeColor(COLOR_COLD);
  } else if (isPinHigh(x) == 1) {
    changeColor(COLOR_WARM);
  } else if (isPinLow(y) == 1) {
    changeColor(COLOR_RED);
  } else if (isPinHigh(y) == 1) {
    changeColor(COLOR_WHITE);
  } else {
    shouldIgnorePin = 0;
  }

  if (didChange == 0) return;

  allOff();

  switch (color) {
    case COLOR_WHITE:
      whiteDesklamp();
      break;
    case COLOR_COLD:
      if (isLampOn == 1) {
        coldWithDesklamp();
      } else {
        cold();
      }
      break;
    case COLOR_WARM:
      if (isLampOn == 1) {
        warmWithDesklamp();
      } else {
        warm();
      }
      break;
    case COLOR_RED:
      if (isLampOn == 1) {
        redWithDesklamp();
      } else {
        lowRed();
      }
      break;
  }

  shouldAnimateTop = 0;
  animationDuration = 0;
  didChange = 0;
}
