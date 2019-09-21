// ******************* НАСТРОЙКИ *****************
// "масштаб" эффектов. Чем меньше, тем крупнее!
#define MADNESS_SCALE 100
#define CLOUD_SCALE 30
#define LAVA_SCALE 50
#define PLASMA_SCALE 30
#define RAINBOW_SCALE 30
#define RAINBOW_S_SCALE 20
#define ZEBRA_SCALE 30
#define FOREST_SCALE 120
#define OCEAN_SCALE 90

// ***************** ДЛЯ РАЗРАБОТЧИКОВ ******************

// The 16 bit version of our coordinates
static uint16_t x;
static uint16_t y;
static uint16_t z;

uint16_t speed = 20; // speed is set dynamically once we've started up
uint16_t scale = 30; // scale is set dynamically once we've started up

// This is the array that we keep our computed noise values in
#define MAX_DIMENSION (max(WIDTH, HEIGHT))
#if (WIDTH > HEIGHT)
uint8_t noise[WIDTH][WIDTH];
#else
uint8_t noise[HEIGHT][HEIGHT];
#endif

CRGBPalette16 currentPalette( PartyColors_p );
uint8_t colorLoop = 1;
uint8_t ihue = 0;

void madnessNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    scale = modes[5].scale;
    speed = modes[5].speed;
  }
  fillnoise8();
  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      CRGB thisColor = CHSV(noise[j][i], 255, noise[i][j]);
      drawPixelXY(i, j, thisColor);   //leds[getPixelNumber(i, j)] = CHSV(noise[j][i], 255, noise[i][j]);
    }
  }
  ihue += 1;
}
void rainbowNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    currentPalette = RainbowColors_p;
    scale = modes[9].scale;
    speed = modes[9].speed;
    colorLoop = 1;
  }
  fillNoiseLED();
}
void rainbowStripeNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    currentPalette = RainbowStripeColors_p;
    scale = modes[10].scale;
    speed = modes[10].speed;
    colorLoop = 1;
  }
  fillNoiseLED();
}
void zebraNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;
    scale = modes[11].scale;
    speed = modes[11].speed;
    colorLoop = 1;
  }
  fillNoiseLED();
}
void forestNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    currentPalette = ForestColors_p;
    scale = modes[12].scale;
    speed = modes[12].speed;
    colorLoop = 0;
  }
  fillNoiseLED();
}
void oceanNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    currentPalette = OceanColors_p;
    scale = modes[13].scale;
    speed = modes[13].speed;
    colorLoop = 0;
  }

  fillNoiseLED();
}
void plasmaNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    currentPalette = PartyColors_p;
    scale = modes[8].scale;
    speed = modes[8].speed;
    colorLoop = 1;
  }
  fillNoiseLED();
}
void cloudNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    currentPalette = CloudColors_p;
    scale = modes[6].scale;
    speed = modes[6].speed;
    colorLoop = 0;
  }
  fillNoiseLED();
}
void lavaNoise() {
  if (loadingFlag) {
    loadingFlag = false;
    currentPalette = LavaColors_p;
    scale = modes[7].scale;
    speed = modes[7].speed;
    colorLoop = 0;
  }
  fillNoiseLED();
}

// ******************* СЛУЖЕБНЫЕ *******************
void fillNoiseLED() {
  uint8_t dataSmoothing = 0;
  if ( speed < 50) {
    dataSmoothing = 200 - (speed * 4);
  }
  for (int i = 0; i < MAX_DIMENSION; i++) {
    int ioffset = scale * i;
    for (int j = 0; j < MAX_DIMENSION; j++) {
      int joffset = scale * j;

      uint8_t data = inoise8(x + ioffset, y + joffset, z);

      data = qsub8(data, 16);
      data = qadd8(data, scale8(data, 39));

      if ( dataSmoothing ) {
        uint8_t olddata = noise[i][j];
        uint8_t newdata = scale8( olddata, dataSmoothing) + scale8( data, 256 - dataSmoothing);
        data = newdata;
      }

      noise[i][j] = data;
    }
  }
  z += speed;

  // apply slow drift to X and Y, just for visual variation.
  x += speed / 8;
  y -= speed / 16;

  for (int i = 0; i < WIDTH; i++) {
    for (int j = 0; j < HEIGHT; j++) {
      uint8_t index = noise[j][i];
      uint8_t bri =   noise[i][j];
      // if this palette is a 'loop', add a slowly-changing base value
      if ( colorLoop) {
        index += ihue;
      }
      // brighten up, as the color palette itself often contains the
      // light/dark dynamic range desired
      if ( bri > 127 ) {
        bri = 255;
      } else {
        bri = dim8_raw( bri * 2);
      }
      CRGB color = ColorFromPalette( currentPalette, index, bri);      
      drawPixelXY(i, j, color);   //leds[getPixelNumber(i, j)] = color;
    }
  }
  ihue += 1;
}

void fillnoise8() {
  for (int i = 0; i < MAX_DIMENSION; i++) {
    int ioffset = scale * i;
    for (int j = 0; j < MAX_DIMENSION; j++) {
      int joffset = scale * j;
      noise[i][j] = inoise8(x + ioffset, y + joffset, z);
    }
  }
  z += speed;
}
