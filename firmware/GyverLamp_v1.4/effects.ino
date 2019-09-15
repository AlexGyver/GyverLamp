// ============= ЭФФЕКТЫ ===============

// ------------- конфетти --------------
#define FADE_OUT_SPEED        (70U)                         // скорость затухания
void sparklesRoutine()
{
  for (uint8_t i = 0; i < modes[EFF_SPARKLES].Scale; i++)
  {
    uint8_t x = random(0, WIDTH);
    uint8_t y = random(0, HEIGHT);
    if (getPixColorXY(x, y) == 0)
    {
      leds[getPixelNumber(x, y)] = CHSV(random(0, 255), 255, 255);
    }
  }
  fader(FADE_OUT_SPEED);
}

// функция плавного угасания цвета для всех пикселей
void fader(uint8_t step)
{
  for (uint8_t i = 0; i < WIDTH; i++)
  {
    for (uint8_t j = 0; j < HEIGHT; j++)
    {
      fadePixel(i, j, step);
    }
  }
}

void fadePixel(uint8_t i, uint8_t j, uint8_t step)          // новый фейдер
{
  int32_t pixelNum = getPixelNumber(i, j);
  if (getPixColor(pixelNum) == 0) return;

  if (leds[pixelNum].r >= 30 ||
      leds[pixelNum].g >= 30 ||
      leds[pixelNum].b >= 30)
  {
    leds[pixelNum].fadeToBlackBy(step);
  }
  else
  {
    leds[pixelNum] = 0;
  }
}

// ------------- огонь -----------------
#define SPARKLES              (1U)                          // вылетающие угольки вкл выкл
uint8_t line[WIDTH];
uint8_t pcnt = 0;

//these values are substracetd from the generated values to give a shape to the animation
const uint8_t valueMask[8][16] PROGMEM =
{
  {32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 },
  {64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 },
  {96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 },
  {128, 64 , 32 , 0  , 0  , 32 , 64 , 128, 128, 64 , 32 , 0  , 0  , 32 , 64 , 128},
  {160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 , 32 , 64 , 96 , 160},
  {192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128, 96 , 64 , 64 , 96 , 128, 192},
  {255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160, 128, 96 , 96 , 128, 160, 255},
  {255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 255}
};

//these are the hues for the fire,
//should be between 0 (red) to about 25 (yellow)
const uint8_t hueMask[8][16] PROGMEM =
{
  {1 , 11, 19, 25, 25, 22, 11, 1 , 1 , 11, 19, 25, 25, 22, 11, 1 },
  {1 , 8 , 13, 19, 25, 19, 8 , 1 , 1 , 8 , 13, 19, 25, 19, 8 , 1 },
  {1 , 8 , 13, 16, 19, 16, 8 , 1 , 1 , 8 , 13, 16, 19, 16, 8 , 1 },
  {1 , 5 , 11, 13, 13, 13, 5 , 1 , 1 , 5 , 11, 13, 13, 13, 5 , 1 },
  {1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11, 11, 11, 5 , 1 },
  {0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 , 0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 },
  {0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 , 0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 },
  {0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 }
};

void fireRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
    generateLine();
    memset(matrixValue, 0, sizeof(matrixValue));
  }
  if (pcnt >= 100)
  {
    shiftUp();
    generateLine();
    pcnt = 0;
  }
  drawFrame(pcnt);
  pcnt += 30;
}

// Randomly generate the next line (matrix row)
void generateLine()
{
  for (uint8_t x = 0; x < WIDTH; x++)
  {
    line[x] = random(64, 255);
  }
}

void shiftUp()
{
  for (uint8_t y = HEIGHT - 1; y > 0; y--)
  {
    for (uint8_t x = 0; x < WIDTH; x++)
    {
      uint8_t newX = x;
      if (x > 15) newX = x % 16;
      if (y > 7) continue;
      matrixValue[y][newX] = matrixValue[y - 1][newX];
    }
  }

  for (uint8_t x = 0; x < WIDTH; x++)
  {
    uint8_t newX = x;
    if (x > 15) newX = x % 16;
    matrixValue[0][newX] = line[newX];
  }
}

// draw a frame, interpolating between 2 "key frames"
// @param pcnt percentage of interpolation

void drawFrame(uint8_t pcnt)
{
  int32_t nextv;

  //each row interpolates with the one before it
  for (uint8_t y = HEIGHT - 1; y > 0; y--)
  {
    for (uint8_t x = 0; x < WIDTH; x++)
    {
      uint8_t newX = x;
      if (x > 15) newX = x % 16;
      if (y < 8)
      {
        nextv =
          (((100.0 - pcnt) * matrixValue[y][newX]
            + pcnt * matrixValue[y - 1][newX]) / 100.0)
          - pgm_read_byte(&(valueMask[y][newX]));

        CRGB color = CHSV(
          modes[EFF_FIRE].Scale * 2.5 + pgm_read_byte(&(hueMask[y][newX])),          // H
          255,                                                                       // S
          (uint8_t)max(0, nextv)                                                     // V
        );

        leds[getPixelNumber(x, y)] = color;
      }
      else if (y == 8 && SPARKLES)
      {
        if (random(0, 20) == 0 && getPixColorXY(x, y - 1) != 0) drawPixelXY(x, y, getPixColorXY(x, y - 1));
        else drawPixelXY(x, y, 0);
      }
      else if (SPARKLES)
      {
        // старая версия для яркости
        if (getPixColorXY(x, y - 1) > 0)
          drawPixelXY(x, y, getPixColorXY(x, y - 1));
        else drawPixelXY(x, y, 0);
      }
    }
  }

  //first row interpolates with the "next" line
  for (uint8_t x = 0; x < WIDTH; x++)
  {
    uint8_t newX = x;
    if (x > 15) newX = x % 16;
    CRGB color = CHSV(
      modes[EFF_FIRE].Scale * 2.5 + pgm_read_byte(&(hueMask[0][newX])),              // H
      255,                                                                           // S
      (uint8_t)(((100.0 - pcnt) * matrixValue[0][newX] + pcnt * line[newX]) / 100.0) // V
    );
    //leds[getPixelNumber(newX, 0)] = color;                                         // на форуме пишут что это ошибка - вместо newX должно быть x, иначе
    leds[getPixelNumber(x, 0)] = color;                                              // на матрицах шире 16 столбцов нижний правый угол неработает
  }
}

// ------------- радуга вертикальная ----------------
uint8_t hue;
void rainbowVerticalRoutine()
{
  hue += 4;
  for (uint8_t j = 0; j < HEIGHT; j++)
  {
    CHSV thisColor = CHSV((uint8_t)(hue + j * modes[EFF_RAINBOW_VER].Scale), 255, 255);
    for (uint8_t i = 0; i < WIDTH; i++)
    {
      drawPixelXY(i, j, thisColor);
    }
  }
}

// ------------- радуга горизонтальная ----------------
void rainbowHorizontalRoutine()
{
  hue += 4;
  for (uint8_t i = 0; i < WIDTH; i++)
  {
    CHSV thisColor = CHSV((uint8_t)(hue + i * modes[EFF_RAINBOW_HOR].Scale), 255, 255);
    for (uint8_t j = 0; j < HEIGHT; j++)
    {
      drawPixelXY(i, j, thisColor);
    }
  }
}

// ------------- радуга дигональная -------------
void rainbowDiagonalRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
  }

  hue += 8;
  for (uint8_t i = 0; i < WIDTH; i++)
  {
    for (uint8_t j = 0; j < HEIGHT; j++)
    {
      float twirlFactor = 3.0F * (modes[EFF_RAINBOW_DIAG].Scale / 100.0F);      // на сколько оборотов будет закручена матрица, [0..3]
      CRGB thisColor = CHSV(constrain((uint8_t)(hue + (float)(WIDTH / HEIGHT * i + j * twirlFactor) * (float)(255 / maxDim)), 0, 255), 255, 255);
      drawPixelXY(i, j, thisColor);
    }
  }
}

// ------------- цвета -----------------
void colorsRoutine()
{
  if (loadingFlag)
  {
    hue += modes[EFF_COLORS].Scale;

    for (int16_t i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(hue, 255, 255);
    }
  }
}

// ------------- цвет ------------------
void colorRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();

    for (int16_t i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(modes[EFF_COLOR].Scale * 2.5, 255, 255);
    }
  }
}

// ------------- снегопад ----------
void snowRoutine()
{
  // сдвигаем всё вниз
  for (uint8_t x = 0; x < WIDTH; x++)
  {
    for (uint8_t y = 0; y < HEIGHT - 1; y++)
    {
      drawPixelXY(x, y, getPixColorXY(x, y + 1));
    }
  }

  for (uint8_t x = 0; x < WIDTH; x++)
  {
    // заполняем случайно верхнюю строку
    // а также не даём двум блокам по вертикали вместе быть
    if (getPixColorXY(x, HEIGHT - 2) == 0 && (random(0, 100 - modes[EFF_SNOW].Scale) == 0))
      drawPixelXY(x, HEIGHT - 1, 0xE0FFFF - 0x101010 * random(0, 4));
    else
      drawPixelXY(x, HEIGHT - 1, 0x000000);
  }
}

// ------------- метель -------------
#define SNOW_DENSE            (60U)                         // плотность снега
#define SNOW_TAIL_STEP        (100U)                        // длина хвоста
#define SNOW_SATURATION       (0U)                          // насыщенность (от 0 до 255)
void snowStormRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
  }
  
  // заполняем головами комет левую и верхнюю линию
  for (uint8_t i = HEIGHT / 2; i < HEIGHT; i++)
  {
    if (getPixColorXY(0, i) == 0 &&
       (random(0, SNOW_DENSE) == 0) &&
        getPixColorXY(0, i + 1) == 0 &&
        getPixColorXY(0, i - 1) == 0)
    {
      leds[getPixelNumber(0, i)] = CHSV(random(0, 200), SNOW_SATURATION, 255);
    }
  }
  
  for (uint8_t i = 0; i < WIDTH / 2; i++)
  {
    if (getPixColorXY(i, HEIGHT - 1) == 0 &&
       (random(0, map(modes[EFF_SNOWSTORM].Scale, 0, 255, 10, 120)) == 0) &&
        getPixColorXY(i + 1, HEIGHT - 1) == 0 &&
        getPixColorXY(i - 1, HEIGHT - 1) == 0)
    {
      leds[getPixelNumber(i, HEIGHT - 1)] = CHSV(random(0, 200), SNOW_SATURATION, 255);
    }
  }

  // сдвигаем по диагонали
  for (uint8_t y = 0; y < HEIGHT - 1; y++)
  {
    for (uint8_t x = WIDTH - 1; x > 0; x--)
    {
      drawPixelXY(x, y, getPixColorXY(x - 1, y + 1));
    }
  }

  // уменьшаем яркость левой и верхней линии, формируем "хвосты"
  for (uint8_t i = HEIGHT / 2; i < HEIGHT; i++)
  {
    fadePixel(0, i, SNOW_TAIL_STEP);
  }
  for (uint8_t i = 0; i < WIDTH / 2; i++)
  {
    fadePixel(i, HEIGHT - 1, SNOW_TAIL_STEP);
  }
}

// ------------- звездопад -------------
#define STAR_DENSE            (60U)                         // плотность комет
#define STAR_TAIL_STEP        (100U)                        // длина хвоста кометы
#define STAR_SATURATION       (150U)                        // насыщенность кометы (от 0 до 255)
void starfallRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
  }
  
  // заполняем головами комет левую и верхнюю линию
  for (uint8_t i = HEIGHT / 2; i < HEIGHT; i++)
  {
    if (getPixColorXY(0, i) == 0 &&
       (random(0, STAR_DENSE) == 0) &&
        getPixColorXY(0, i + 1) == 0 &&
        getPixColorXY(0, i - 1) == 0)
    {
      leds[getPixelNumber(0, i)] = CHSV(random(0, 200), STAR_SATURATION, 255);
    }
  }
  
  for (uint8_t i = 0; i < WIDTH / 2; i++)
  {
    if (getPixColorXY(i, HEIGHT - 1) == 0 &&
       (random(0, map(modes[EFF_STARFALL].Scale, 0, 255, 10, 120)) == 0) &&
        getPixColorXY(i + 1, HEIGHT - 1) == 0 &&
        getPixColorXY(i - 1, HEIGHT - 1) == 0)
    {
      leds[getPixelNumber(i, HEIGHT - 1)] = CHSV(random(0, 200), STAR_SATURATION, 255);
    }
  }

  // сдвигаем по диагонали
  for (uint8_t y = 0; y < HEIGHT - 1; y++)
  {
    for (uint8_t x = WIDTH - 1; x > 0; x--)
    {
      drawPixelXY(x, y, getPixColorXY(x - 1, y + 1));
    }
  }

  // уменьшаем яркость левой и верхней линии, формируем "хвосты"
  for (uint8_t i = HEIGHT / 2; i < HEIGHT; i++)
  {
    fadePixel(0, i, STAR_TAIL_STEP);
  }
  for (uint8_t i = 0; i < WIDTH / 2; i++)
  {
    fadePixel(i, HEIGHT - 1, STAR_TAIL_STEP);
  }
}

// ------------- матрица ---------------
void matrixRoutine()
{
  for (uint8_t x = 0; x < WIDTH; x++)
  {
    // заполняем случайно верхнюю строку
    uint32_t thisColor = getPixColorXY(x, HEIGHT - 1);
    if (thisColor == 0)
      drawPixelXY(x, HEIGHT - 1, 0x00FF00 * (random(0, 100 - modes[EFF_MATRIX].Scale) == 0));
    else if (thisColor < 0x002000)
      drawPixelXY(x, HEIGHT - 1, 0);
    else
      drawPixelXY(x, HEIGHT - 1, thisColor - 0x002000);
  }

  // сдвигаем всё вниз
  for (uint8_t x = 0; x < WIDTH; x++)
  {
    for (uint8_t y = 0; y < HEIGHT - 1; y++)
    {
      drawPixelXY(x, y, getPixColorXY(x, y + 1));
    }
  }
}

// ------------- светлячки --------------
#define LIGHTERS_AM           (100)
int32_t lightersPos[2][LIGHTERS_AM];
int8_t lightersSpeed[2][LIGHTERS_AM];
CHSV lightersColor[LIGHTERS_AM];
uint8_t loopCounter;
int32_t angle[LIGHTERS_AM];
int32_t speedV[LIGHTERS_AM];
int8_t angleSpeed[LIGHTERS_AM];
void lightersRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    randomSeed(millis());
    for (uint8_t i = 0; i < LIGHTERS_AM; i++)
    {
      lightersPos[0][i] = random(0, WIDTH * 10);
      lightersPos[1][i] = random(0, HEIGHT * 10);
      lightersSpeed[0][i] = random(-10, 10);
      lightersSpeed[1][i] = random(-10, 10);
      lightersColor[i] = CHSV(random(0, 255), 255, 255);
    }
  }
  FastLED.clear();
  if (++loopCounter > 20) loopCounter = 0;
  for (uint8_t i = 0; i < modes[EFF_LIGHTERS].Scale; i++)
  {
    if (loopCounter == 0)                                   // меняем скорость каждые 255 отрисовок
    {
      lightersSpeed[0][i] += random(-3, 4);
      lightersSpeed[1][i] += random(-3, 4);
      lightersSpeed[0][i] = constrain(lightersSpeed[0][i], -20, 20);
      lightersSpeed[1][i] = constrain(lightersSpeed[1][i], -20, 20);
    }

    lightersPos[0][i] += lightersSpeed[0][i];
    lightersPos[1][i] += lightersSpeed[1][i];

    if (lightersPos[0][i] < 0) lightersPos[0][i] = (WIDTH - 1) * 10;
    if (lightersPos[0][i] >= WIDTH * 10) lightersPos[0][i] = 0;

    if (lightersPos[1][i] < 0)
    {
      lightersPos[1][i] = 0;
      lightersSpeed[1][i] = -lightersSpeed[1][i];
    }
    if (lightersPos[1][i] >= (HEIGHT - 1) * 10)
    {
      lightersPos[1][i] = (HEIGHT - 1) * 10;
      lightersSpeed[1][i] = -lightersSpeed[1][i];
    }
    drawPixelXY(lightersPos[0][i] / 10, lightersPos[1][i] / 10, lightersColor[i]);
  }
}

// ------------- светлячки со шлейфом -------------
#define BALLS_AMOUNT          (3U)                          // количество "шариков"
#define CLEAR_PATH            (1U)                          // очищать путь
#define BALL_TRACK            (1U)                          // (0 / 1) - вкл/выкл следы шариков
#define TRACK_STEP            (70U)                         // длина хвоста шарика (чем больше цифра, тем хвост короче)
int16_t coord[BALLS_AMOUNT][2];
int8_t vector[BALLS_AMOUNT][2];
CRGB ballColors[BALLS_AMOUNT];
void ballsRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;

    for (uint8_t j = 0; j < BALLS_AMOUNT; j++)
    {
      int8_t sign;
      // забиваем случайными данными
      coord[j][0] = WIDTH / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][0] = random(4, 15) * sign;
      coord[j][1] = HEIGHT / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][1] = random(4, 15) * sign;
      ballColors[j] = CHSV(random(0, 9) * 28, 255, 255);
    }
  }

  if (!BALL_TRACK)                                          // режим без следов шариков
  {
    FastLED.clear();
  }
  else                                                      // режим со следами
  {
    fader(TRACK_STEP);
  }

  // движение шариков
  for (uint8_t j = 0; j < BALLS_AMOUNT; j++)
  {
    // движение шариков
    for (uint8_t i = 0; i < 2; i++)
    {
      coord[j][i] += vector[j][i];
      if (coord[j][i] < 0)
      {
        coord[j][i] = 0;
        vector[j][i] = -vector[j][i];
      }
    }

    if (coord[j][0] > (WIDTH - 1) * 10)
    {
      coord[j][0] = (WIDTH - 1) * 10;
      vector[j][0] = -vector[j][0];
    }
    if (coord[j][1] > (HEIGHT - 1) * 10)
    {
      coord[j][1] = (HEIGHT - 1) * 10;
      vector[j][1] = -vector[j][1];
    }
    leds[getPixelNumber(coord[j][0] / 10, coord[j][1] / 10)] =  ballColors[j];
  }
}

// ------------- пейнтбол -------------
const uint8_t BorderWidth = 2;
void lightBallsRoutine()
{
  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly.  Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
  uint8_t blurAmount = dim8_raw(beatsin8(3,64,100));
  blur2d(leds, WIDTH, HEIGHT, blurAmount);

  // Use two out-of-sync sine waves
  uint8_t  i = beatsin8(  91, BorderWidth, WIDTH-BorderWidth);
  uint8_t  j = beatsin8( 109, BorderWidth, WIDTH-BorderWidth);
  uint8_t  k = beatsin8(  73, BorderWidth, WIDTH-BorderWidth);
  uint8_t  m = beatsin8( 123, BorderWidth, WIDTH-BorderWidth);

  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis();
  leds[XY( i, j)] += CHSV( ms / 29, 200, 255);
  leds[XY( j, k)] += CHSV( ms / 41, 200, 255);
  leds[XY( k, m)] += CHSV( ms / 73, 200, 255);
  leds[XY( m, i)] += CHSV( ms / 97, 200, 255);

}
// Trivial XY function for the SmartMatrix; use a different XY
// function for different matrix grids. See XYMatrix example for code.
uint16_t XY(uint8_t x, uint8_t y)
{
  uint16_t i;
  if (y & 0x01)
  {
    // Odd rows run backwards
    uint8_t reverseX = (WIDTH - 1) - x;
    i = (y * WIDTH) + reverseX;
  }
  else
  {
    // Even rows run forwards
    i = (y * WIDTH) + x;
  }
  return i;
}

// ------------- блуждающий кубик -------------
#define RANDOM_COLOR          (1U)                          // случайный цвет при отскоке
int16_t coordB[2];
int8_t vectorB[2];
CRGB ballColor;
int8_t ballSize;
void ballRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    //FastLED.clear();

    for (uint8_t i = 0; i < 2; i++)
    {
      coordB[i] = WIDTH / 2 * 10;
      vectorB[i] = random(8, 20);
      ballColor = CHSV(random(0, 9) * 28, 255, 255);
    }
  }

  ballSize = map(modes[EFF_CUBE].Scale, 0, 255, 2, max((int16_t)min(WIDTH,HEIGHT) / 3, 2));
  for (uint8_t i = 0; i < 2; i++)
  {
    coordB[i] += vectorB[i];
    if (coordB[i] < 0)
    {
      coordB[i] = 0;
      vectorB[i] = -vectorB[i];
      if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255, 255);
      //vectorB[i] += random(0, 6) - 3;
    }
  }
  if (coordB[0] > (WIDTH - ballSize) * 10)
  {
    coordB[0] = (WIDTH - ballSize) * 10;
    vectorB[0] = -vectorB[0];
    if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255, 255);
    //vectorB[0] += random(0, 6) - 3;
  }
  if (coordB[1] > (HEIGHT - ballSize) * 10)
  {
    coordB[1] = (HEIGHT - ballSize) * 10;
    vectorB[1] = -vectorB[1];
    if (RANDOM_COLOR)
    {
      ballColor = CHSV(random(0, 9) * 28, 255, 255);
    }
    //vectorB[1] += random(0, 6) - 3;
  }
  FastLED.clear();
  for (uint8_t i = 0; i < ballSize; i++)
  {
    for (uint8_t j = 0; j < ballSize; j++)
    {
      leds[getPixelNumber(coordB[0] / 10 + i, coordB[1] / 10 + j)] = ballColor;
    }
  }
}

// ------------- белый свет -------------
void whiteColorRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();

    for (int16_t i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(0, 0, 255);
    }
  }
}

// ------------- экспериментальный эффект -------------
// ------------- белый свет (уменьшение яркости по горизинтали от центра вверх и вниз; масштаб - ширина центральной полосы максимальной яркости; яркость - общая яркость -------------
#define MINIMUM_BRIGHTNES     (90U)                         // минимальная яркость (ниже матрица мерцает)
void whiteColorRoutine2()
{
  if (loadingFlag)
  {
    Serial.println("Отрисовка");
    loadingFlag = false;
    FastLED.clear();
    delay(1);

    uint8_t centerY = max((uint8_t)round(HEIGHT / 2.0F) - 1, 0);
    uint8_t bottomOffset = (uint8_t)(!(HEIGHT & 1) && (HEIGHT > 1));                      // если высота матрицы чётная, линий с максимальной яркостью две, а линии с угасающей яркостью снизу будут смещены на один ряд
    for (int16_t y = centerY; y >= 0; y--)
    {
      CRGB color = CHSV(100, 1,
        constrain(uint8_t(                                                                // определяем яркость
          modes[EFF_WHITE_COLOR].Brightness * (y + 1) / (centerY + 1) +                   // влияние координаты Y на яркость (плавное затухаие вверх и вниз)
          modes[EFF_WHITE_COLOR].Brightness * modes[EFF_WHITE_COLOR].Scale / 100),        // влияние масштаба на яркость
          MINIMUM_BRIGHTNES,
          max((uint8_t)MINIMUM_BRIGHTNES, (uint8_t)modes[EFF_WHITE_COLOR].Brightness)
        ));

      for (uint8_t x = 0; x < WIDTH; x++)
      {
        drawPixelXY(x, y, color);                                                         // при чётной высоте матрицы максимально яркими отрисуются 2 центральных горизонтальных полосы
        drawPixelXY(x, max((uint8_t)(HEIGHT - 1) - (y + 1) + bottomOffset, 0), color);    // при нечётной - одна, но дважды
      }
    }
  }
}

/*
 * устарело
void lightersRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    randomSeed(millis());
    for (uint8_t i = 0; i < LIGHTERS_AM; i++)
    {
      lightersPos[0][i] = random(0, WIDTH * 10);
      lightersPos[1][i] = random(0, HEIGHT * 10);

      lightersColor[i] = CHSV(random(0, 255), 255, 255);

      speedV[i] = random(5, 10);
      angle[i] = random(0, 360);
      angleSpeed[i] = random(-10, 10);
    }
  }
  FastLED.clear();
  if (++loopCounter > 20) loopCounter = 0;

  for (uint8_t i = 0; i < modes[EFF_LIGHTER_TRACES].scale; i++)
  {
    if (loopCounter == 0)                                   // меняем скорость каждые 255 отрисовок
    {
      angleSpeed[i] += random(-3, 4);
      angleSpeed[i] = constrain(angleSpeed[i], -15, 15);
    }

    lightersPos[0][i] += speedV[i] * cos(radians(angle[i]));
    lightersPos[1][i] += speedV[i] * sin(radians(angle[i]));

    if (lightersPos[0][i] < 0) lightersPos[0][i] = (WIDTH - 1) * 10;
    if (lightersPos[0][i] >= WIDTH * 10) lightersPos[0][i] = 0;

    if (lightersPos[1][i] < 0)
    {
      lightersPos[1][i] = 0;
      angle[i] = 360 - angle[i];
    }
    else
    {
      angle[i] += angleSpeed[i];
    }

    if (lightersPos[1][i] >= (HEIGHT - 1) * 10)
    {
      lightersPos[1][i] = (HEIGHT - 1) * 10;
      angle[i] = 360 - angle[i];
    }
    else
    {
      angle[i] += angleSpeed[i];
    }

    if (angle[i] > 360) angle[i] = 360 - angle[i];
    if (angle[i] < 0) angle[i] = 360 + angle[i];

    drawPixelXY(lightersPos[0][i] / 10, lightersPos[1][i] / 10, lightersColor[i]);
  }
}
*/
