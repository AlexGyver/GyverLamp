// ============= ЭФФЕКТЫ ===============

// ------------- конфетти --------------
#define FADE_OUT_SPEED        (70U)                         // скорость затухания
void sparklesRoutine()
{
  for (uint8_t i = 0; i < modes[EFF_SPARKLES].Scale; i++)
  {
    uint8_t x = random(0U, WIDTH);
    uint8_t y = random(0U, HEIGHT);
    if (getPixColorXY(x, y) == 0U)
    {
      leds[getPixelNumber(x, y)] = CHSV(random(0U, 255U), 255U, 255U);
    }
  }
  fader(FADE_OUT_SPEED);
}

// функция плавного угасания цвета для всех пикселей
void fader(uint8_t step)
{
  for (uint8_t i = 0U; i < WIDTH; i++)
  {
    for (uint8_t j = 0U; j < HEIGHT; j++)
    {
      fadePixel(i, j, step);
    }
  }
}

void fadePixel(uint8_t i, uint8_t j, uint8_t step)          // новый фейдер
{
  int32_t pixelNum = getPixelNumber(i, j);
  if (getPixColor(pixelNum) == 0U) return;

  if (leds[pixelNum].r >= 30U ||
      leds[pixelNum].g >= 30U ||
      leds[pixelNum].b >= 30U)
  {
    leds[pixelNum].fadeToBlackBy(step);
  }
  else
  {
    leds[pixelNum] = 0U;
  }
}

// ------------- огонь -----------------
#define SPARKLES              (1U)                          // вылетающие угольки вкл выкл
uint8_t line[WIDTH];
uint8_t pcnt = 0U;

//these values are substracetd from the generated values to give a shape to the animation
static const uint8_t valueMask[8][16] PROGMEM =
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
static const uint8_t hueMask[8][16] PROGMEM =
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

void fireRoutine(bool isColored)                            // true - цветной огонь, false - белый
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
  drawFrame(pcnt, isColored);
  pcnt += 30;
}

// Randomly generate the next line (matrix row)
void generateLine()
{
  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    line[x] = random(64, 255);
  }
}

void shiftUp()
{
  for (uint8_t y = HEIGHT - 1U; y > 0U; y--)
  {
    for (uint8_t x = 0U; x < WIDTH; x++)
    {
      uint8_t newX = x;
      if (x > 15U) newX = x % 16U;
      if (y > 7U) continue;
      matrixValue[y][newX] = matrixValue[y - 1U][newX];
    }
  }

  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    uint8_t newX = x;
    if (x > 15U) newX = x % 16U;
    matrixValue[0U][newX] = line[newX];
  }
}

// draw a frame, interpolating between 2 "key frames"
// @param pcnt percentage of interpolation

void drawFrame(uint8_t pcnt, bool isColored)
{
  int32_t nextv;

  //each row interpolates with the one before it
  for (uint8_t y = HEIGHT - 1U; y > 0U; y--)
  {
    for (uint8_t x = 0U; x < WIDTH; x++)
    {
      uint8_t newX = x;
      if (x > 15U) newX = x % 16U;
      if (y < 8U)
      {
        nextv =
          (((100.0 - pcnt) * matrixValue[y][newX]
            + pcnt * matrixValue[y - 1][newX]) / 100.0)
          - pgm_read_byte(&valueMask[y][newX]);

        CRGB color = CHSV(
          isColored ? modes[EFF_FIRE].Scale * 2.5 + pgm_read_byte(&hueMask[y][newX]) : 0U,     // H
          isColored ? 255U : 0U,                                                               // S
          (uint8_t)max(0, nextv)                                                               // V
        );

        leds[getPixelNumber(x, y)] = color;
      }
      else if (y == 8U && SPARKLES)
      {
        if (random(0, 20) == 0 && getPixColorXY(x, y - 1U) != 0U) drawPixelXY(x, y, getPixColorXY(x, y - 1U));
        else drawPixelXY(x, y, 0U);
      }
      else if (SPARKLES)
      {
        // старая версия для яркости
        if (getPixColorXY(x, y - 1U) > 0U)
          drawPixelXY(x, y, getPixColorXY(x, y - 1U));
        else drawPixelXY(x, y, 0U);
      }
    }
  }

  //first row interpolates with the "next" line
  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    uint8_t newX = x;
    if (x > 15U) newX = x % 16U;
    CRGB color = CHSV(
      isColored ? modes[EFF_FIRE].Scale * 2.5 + pgm_read_byte(&(hueMask[0][newX])): 0U,        // H
      isColored ? 255U : 0U,                                                                   // S
      (uint8_t)(((100.0 - pcnt) * matrixValue[0][newX] + pcnt * line[newX]) / 100.0)           // V
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
    for (uint8_t i = 0U; i < WIDTH; i++)
    {
      drawPixelXY(i, j, thisColor);
    }
  }
}

// ------------- радуга горизонтальная ----------------
void rainbowHorizontalRoutine()
{
  hue += 4;
  for (uint8_t i = 0U; i < WIDTH; i++)
  {
    CHSV thisColor = CHSV((uint8_t)(hue + i * modes[EFF_RAINBOW_HOR].Scale), 255, 255);
    for (uint8_t j = 0U; j < HEIGHT; j++)
    {
      drawPixelXY(i, j, thisColor);
    }
  }
}

// ------------- радуга диагональная -------------
void rainbowDiagonalRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
  }

  hue += 8;
  for (uint8_t i = 0U; i < WIDTH; i++)
  {
    for (uint8_t j = 0U; j < HEIGHT; j++)
    {
      float twirlFactor = 3.0F * (modes[EFF_RAINBOW_DIAG].Scale / 100.0F);      // на сколько оборотов будет закручена матрица, [0..3]
      CRGB thisColor = CHSV((uint8_t)(hue + (float)(WIDTH / HEIGHT * i + j * twirlFactor) * (float)(255 / maxDim)), 255, 255);
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

    for (uint16_t i = 0U; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(hue, 255U, 255U);
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

    for (int16_t i = 0U; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(modes[EFF_COLOR].Scale * 2.5, 255U, 255U);
    }
  }
}

// ------------- снегопад ----------
void snowRoutine()
{
  // сдвигаем всё вниз
  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    for (uint8_t y = 0U; y < HEIGHT - 1; y++)
    {
      drawPixelXY(x, y, getPixColorXY(x, y + 1U));
    }
  }

  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    // заполняем случайно верхнюю строку
    // а также не даём двум блокам по вертикали вместе быть
    if (getPixColorXY(x, HEIGHT - 2U) == 0U && (random(0, 100 - modes[EFF_SNOW].Scale) == 0U))
      drawPixelXY(x, HEIGHT - 1U, 0xE0FFFF - 0x101010 * random(0, 4));
    else
      drawPixelXY(x, HEIGHT - 1U, 0x000000);
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
  for (uint8_t i = HEIGHT / 2U; i < HEIGHT; i++)
  {
    if (getPixColorXY(0U, i) == 0U &&
       (random(0, SNOW_DENSE) == 0) &&
        getPixColorXY(0U, i + 1U) == 0U &&
        getPixColorXY(0U, i - 1U) == 0U)
    {
      leds[getPixelNumber(0U, i)] = CHSV(random(0, 200), SNOW_SATURATION, 255U);
    }
  }
  
  for (uint8_t i = 0U; i < WIDTH / 2U; i++)
  {
    if (getPixColorXY(i, HEIGHT - 1U) == 0U &&
       (random(0, map(modes[EFF_SNOWSTORM].Scale, 0U, 255U, 10U, 120U)) == 0U) &&
        getPixColorXY(i + 1U, HEIGHT - 1U) == 0U &&
        getPixColorXY(i - 1U, HEIGHT - 1U) == 0U)
    {
      leds[getPixelNumber(i, HEIGHT - 1U)] = CHSV(random(0, 200), SNOW_SATURATION, 255U);
    }
  }

  // сдвигаем по диагонали
  for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
  {
    for (uint8_t x = WIDTH - 1U; x > 0U; x--)
    {
      drawPixelXY(x, y, getPixColorXY(x - 1U, y + 1U));
    }
  }

  // уменьшаем яркость левой и верхней линии, формируем "хвосты"
  for (uint8_t i = HEIGHT / 2U; i < HEIGHT; i++)
  {
    fadePixel(0U, i, SNOW_TAIL_STEP);
  }
  for (uint8_t i = 0U; i < WIDTH / 2U; i++)
  {
    fadePixel(i, HEIGHT - 1U, SNOW_TAIL_STEP);
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
  for (uint8_t i = HEIGHT / 2U; i < HEIGHT; i++)
  {
    if (getPixColorXY(0U, i) == 0U &&
       (random(0, STAR_DENSE) == 0) &&
        getPixColorXY(0U, i + 1U) == 0U &&
        getPixColorXY(0U, i - 1U) == 0U)
    {
      leds[getPixelNumber(0U, i)] = CHSV(random(0, 200), STAR_SATURATION, 255U);
    }
  }
  
  for (uint8_t i = 0U; i < WIDTH / 2U; i++)
  {
    if (getPixColorXY(i, HEIGHT - 1U) == 0U &&
       (random(0, map(modes[EFF_STARFALL].Scale, 0U, 255U, 10U, 120U)) == 0U) &&
        getPixColorXY(i + 1U, HEIGHT - 1U) == 0U &&
        getPixColorXY(i - 1U, HEIGHT - 1U) == 0U)
    {
      leds[getPixelNumber(i, HEIGHT - 1U)] = CHSV(random(0, 200), STAR_SATURATION, 255U);
    }
  }

  // сдвигаем по диагонали
  for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
  {
    for (uint8_t x = WIDTH - 1U; x > 0U; x--)
    {
      drawPixelXY(x, y, getPixColorXY(x - 1U, y + 1U));
    }
  }

  // уменьшаем яркость левой и верхней линии, формируем "хвосты"
  for (uint8_t i = HEIGHT / 2U; i < HEIGHT; i++)
  {
    fadePixel(0U, i, STAR_TAIL_STEP);
  }
  for (uint8_t i = 0U; i < WIDTH / 2U; i++)
  {
    fadePixel(i, HEIGHT - 1U, STAR_TAIL_STEP);
  }
}

// ------------- матрица ---------------
void matrixRoutine()
{
  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    // заполняем случайно верхнюю строку
    uint32_t thisColor = getPixColorXY(x, HEIGHT - 1U);
    if (thisColor == 0U)
      drawPixelXY(x, HEIGHT - 1U, 0x00FF00 * (random(0, 100 - modes[EFF_MATRIX].Scale) == 0U));
    else if (thisColor < 0x002000)
      drawPixelXY(x, HEIGHT - 1U, 0U);
    else
      drawPixelXY(x, HEIGHT - 1U, thisColor - 0x002000);
  }

  // сдвигаем всё вниз
  for (uint8_t x = 0U; x < WIDTH; x++)
  {
    for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
    {
      drawPixelXY(x, y, getPixColorXY(x, y + 1U));
    }
  }
}

// ------------- светлячки --------------
#define LIGHTERS_AM           (100U)
int32_t lightersPos[2U][LIGHTERS_AM];
int8_t lightersSpeed[2U][LIGHTERS_AM];
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
    for (uint8_t i = 0U; i < LIGHTERS_AM; i++)
    {
      lightersPos[0U][i] = random(0, WIDTH * 10);
      lightersPos[1U][i] = random(0, HEIGHT * 10);
      lightersSpeed[0U][i] = random(-10, 10);
      lightersSpeed[1U][i] = random(-10, 10);
      lightersColor[i] = CHSV(random(0U, 255U), 255U, 255U);
    }
  }
  FastLED.clear();
  if (++loopCounter > 20U) loopCounter = 0U;
  for (uint8_t i = 0U; i < modes[EFF_LIGHTERS].Scale; i++)
  {
    if (loopCounter == 0U)                                  // меняем скорость каждые 255 отрисовок
    {
      lightersSpeed[0U][i] += random(-3, 4);
      lightersSpeed[1U][i] += random(-3, 4);
      lightersSpeed[0U][i] = constrain(lightersSpeed[0U][i], -20, 20);
      lightersSpeed[1U][i] = constrain(lightersSpeed[1U][i], -20, 20);
    }

    lightersPos[0U][i] += lightersSpeed[0U][i];
    lightersPos[1U][i] += lightersSpeed[1U][i];

    if (lightersPos[0U][i] < 0) lightersPos[0U][i] = (WIDTH - 1) * 10;
    if (lightersPos[0U][i] >= (int32_t)(WIDTH * 10)) lightersPos[0U][i] = 0;

    if (lightersPos[1U][i] < 0)
    {
      lightersPos[1U][i] = 0;
      lightersSpeed[1U][i] = -lightersSpeed[1U][i];
    }
    if (lightersPos[1U][i] >= (int32_t)(HEIGHT - 1) * 10)
    {
      lightersPos[1U][i] = (HEIGHT - 1U) * 10;
      lightersSpeed[1U][i] = -lightersSpeed[1U][i];
    }
    drawPixelXY(lightersPos[0U][i] / 10, lightersPos[1U][i] / 10, lightersColor[i]);
  }
}

// ------------- светлячки со шлейфом -------------
#define BALLS_AMOUNT          (3U)                          // количество "шариков"
#define CLEAR_PATH            (1U)                          // очищать путь
#define BALL_TRACK            (1U)                          // (0 / 1) - вкл/выкл следы шариков
#define TRACK_STEP            (70U)                         // длина хвоста шарика (чем больше цифра, тем хвост короче)
int16_t coord[BALLS_AMOUNT][2U];
int8_t vector[BALLS_AMOUNT][2U];
CRGB ballColors[BALLS_AMOUNT];
void ballsRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;

    for (uint8_t j = 0U; j < BALLS_AMOUNT; j++)
    {
      int8_t sign;
      // забиваем случайными данными
      coord[j][0U] = WIDTH / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][0U] = random(4, 15) * sign;
      coord[j][1U] = HEIGHT / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][1U] = random(4, 15) * sign;
      ballColors[j] = CHSV(random(0, 9) * 28, 255U, 255U);
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
  for (uint8_t j = 0U; j < BALLS_AMOUNT; j++)
  {
    // движение шариков
    for (uint8_t i = 0U; i < 2U; i++)
    {
      coord[j][i] += vector[j][i];
      if (coord[j][i] < 0)
      {
        coord[j][i] = 0;
        vector[j][i] = -vector[j][i];
      }
    }

    if (coord[j][0U] > (int16_t)((WIDTH - 1) * 10))
    {
      coord[j][0U] = (WIDTH - 1) * 10;
      vector[j][0U] = -vector[j][0U];
    }
    if (coord[j][1U] > (int16_t)((HEIGHT - 1) * 10))
    {
      coord[j][1U] = (HEIGHT - 1) * 10;
      vector[j][1U] = -vector[j][1U];
    }
    leds[getPixelNumber(coord[j][0U] / 10, coord[j][1U] / 10)] =  ballColors[j];
  }
}

// ------------- пейнтбол -------------
const uint8_t BorderWidth = 2U;
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
  leds[XY( i, j)] += CHSV( ms / 29, 200U, 255U);
  leds[XY( j, k)] += CHSV( ms / 41, 200U, 255U);
  leds[XY( k, m)] += CHSV( ms / 73, 200U, 255U);
  leds[XY( m, i)] += CHSV( ms / 97, 200U, 255U);

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
int16_t coordB[2U];
int8_t vectorB[2U];
CRGB ballColor;
int8_t ballSize;
void ballRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    //FastLED.clear();

    for (uint8_t i = 0U; i < 2U; i++)
    {
      coordB[i] = WIDTH / 2 * 10;
      vectorB[i] = random(8, 20);
      ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
    }
  }

  ballSize = map(modes[EFF_CUBE].Scale, 0U, 255U, 2U, max((uint8_t)min(WIDTH,HEIGHT) / 3, 2));
  for (uint8_t i = 0U; i < 2U; i++)
  {
    coordB[i] += vectorB[i];
    if (coordB[i] < 0)
    {
      coordB[i] = 0;
      vectorB[i] = -vectorB[i];
      if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
      //vectorB[i] += random(0, 6) - 3;
    }
  }
  if (coordB[0U] > (int16_t)((WIDTH - ballSize) * 10))
  {
    coordB[0U] = (WIDTH - ballSize) * 10;
    vectorB[0U] = -vectorB[0U];
    if (RANDOM_COLOR) ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
    //vectorB[0] += random(0, 6) - 3;
  }
  if (coordB[1U] > (int16_t)((HEIGHT - ballSize) * 10))
  {
    coordB[1U] = (HEIGHT - ballSize) * 10;
    vectorB[1U] = -vectorB[1U];
    if (RANDOM_COLOR)
    {
      ballColor = CHSV(random(0, 9) * 28, 255U, 255U);
    }
    //vectorB[1] += random(0, 6) - 3;
  }
  FastLED.clear();
  for (uint8_t i = 0U; i < ballSize; i++)
  {
    for (uint8_t j = 0U; j < ballSize; j++)
    {
      leds[getPixelNumber(coordB[0U] / 10 + i, coordB[1U] / 10 + j)] = ballColor;
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

    for (uint16_t i = 0U; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(0U, 0U, 255U);
    }
  }
}

// ------------- белый свет (светится горизонтальная полоса по центру лампы; масштаб - высота центральной горизонтальной полосы; скорость - регулировка от холодного к тёплому; яркость - общая яркость) -------------
void whiteColorStripeRoutine()
{
  if (loadingFlag)
  {
    loadingFlag = false;
    FastLED.clear();
    delay(1);

    uint8_t centerY = max((uint8_t)round(HEIGHT / 2.0F) - 1, 0);
    uint8_t bottomOffset = (uint8_t)(!(HEIGHT & 1) && (HEIGHT > 1));                      // если высота матрицы чётная, линий с максимальной яркостью две, а линии с минимальной яркостью снизу будут смещены на один ряд
    for (int16_t y = centerY; y >= 0; y--)
    {
      CRGB color = CHSV(
        45U,                                                                              // определяем тон
        map(modes[EFF_WHITE_COLOR].Speed, 0U, 255U, 0U, 170U),                            // определяем насыщенность
        y == centerY                                                                                                    // определяем яркость
          ? 255U                                                                                                        // для центральной горизонтальной полосы (или двух) яркость всегда равна 255
          : (modes[EFF_WHITE_COLOR].Scale / 100.0F) > ((centerY + 1.0F) - (y + 1.0F)) / (centerY + 1.0F) ? 255U : 0U);  // для остальных горизонтальных полос яркость равна либо 255, либо 0 в зависимости от масштаба

      for (uint8_t x = 0U; x < WIDTH; x++)
      {
        drawPixelXY(x, y, color);                                                         // при чётной высоте матрицы максимально яркими отрисуются 2 центральных горизонтальных полосы
        drawPixelXY(x, max((uint8_t)(HEIGHT - 1U) - (y + 1U) + bottomOffset, 0U), color); // при нечётной - одна, но дважды
      }
    }
  }
}

// ------------- мигающий цвет (не эффект! используется для отображения краткосрочного предупреждения; блокирующий код!) -------------
#define WARNING_BRIGHTNESS    (10U)                         // яркость вспышки
void showWarning(
  CRGB color,                                               /* цвет вспышки                                                 */
  uint32_t duration,                                        /* продолжительность отображения предупреждения (общее время)   */
  uint16_t blinkHalfPeriod)                                 /* продолжительность одной вспышки в миллисекундах (полупериод) */
{
  uint32_t blinkTimer = millis();
  enum BlinkState { OFF = 0, ON = 1 } blinkState = BlinkState::OFF;
  FastLED.setBrightness(WARNING_BRIGHTNESS);                // установка яркости для предупреждения
  FastLED.clear();
  delay(2);
  FastLED.show();

  for (uint16_t i = 0U; i < NUM_LEDS; i++)                  // установка цвета всех диодов в WARNING_COLOR
  {
    leds[i] = color;
  }

  uint32_t startTime = millis();
  while (millis() - startTime <= (duration + 5))            // блокировка дальнейшего выполнения циклом на время отображения предупреждения
  {
    if (millis() - blinkTimer >= blinkHalfPeriod)           // переключение вспышка/темнота
    {
      blinkTimer = millis();
      blinkState = (BlinkState)!blinkState;
      FastLED.setBrightness(blinkState == BlinkState::OFF ? 0 : WARNING_BRIGHTNESS);
      delay(1);
      FastLED.show();
    }
    delay(50);
  }

  FastLED.clear();
  FastLED.setBrightness(ONflag ? modes[currentMode].Brightness : 0);  // установка яркости, которая была выставлена до вызова предупреждения
  delay(1);
  FastLED.show();
  loadingFlag = true;                                       // принудительное отображение текущего эффекта (того, что был активен перед предупреждением)
}
