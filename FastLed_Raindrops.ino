#include "FastSPI_LED2.h"

#define DATA_PIN 2
#define rowCount 14
#define ledCount 112

// This is an array of leds.  One item for each led in your strip.
CRGB leds[ledCount];

enum mode {modeSunrise, modeSunset, modeDay, modeNight, modeDayStorm, modeNightStorm, modeRainbow, modeRaindrop};
mode currentMode = modeRaindrop;

byte rain[rowCount] = {0};

#ifndef __AVR_ATtiny85__
  const byte btnRainbow = 6;
  const byte btnChangeDay = 7;
#endif

#ifdef  __AVR_ATtiny85__
  const byte btnRainbow = 0;
  const byte btnChangeDay = 1;  
#endif  


void setup() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, ledCount);

  pinMode(btnRainbow, INPUT);
  pinMode(btnChangeDay, INPUT);
  
//  Serial.begin(9600);
}

void loop()
{
  switch(currentMode)
  {
    /*
    case modeSunrise:
      sunrise();
      break;
    case modeSunset:
      sunset();
      break;
      */
    case modeDay:
      calm(true);
      break;
    case modeNight:
      calm(false);
      break;
    case modeDayStorm:
      storm(); // daystorm
      break;
    case modeNightStorm:
      storm(); // daystorm
      break;
      /*
    case modeRainbow:
      rainbowLoop();
      break;
      */
    case modeRaindrop:
      animateRain();
      break;
    default:
      break;
  }
  
  //checkForEvent();
}

void rainbowLoop() {              //-m3-LOOP HSV RAINBOW
  static byte hue = 0;
  static int currentFrame = 0;
  currentFrame++;
  //print(currentFrame);println();
  
  fill_rainbow(leds, ledCount, hue++, 10);
  FastLED.show();
  delay(15);
  hue++;
  if(currentFrame == 1024)
  {
    currentMode = modeDay;
  }
}

// there are 8 leds strip so I'm using a byte with 
// one bit to represent each column and then I have
// 14 of them since each strip is 14 pixels long
//
// in seedDrops I am randomly starting raindrops
// by setting the first bit true in some of the strips.
void seedDrop()
{
  //print("seed drop ");println();
  for(byte iColumn = 0;iColumn<8;iColumn++)
  {
    if (random(0,8) == 0) 
    {
      //print("1");  
      bitSet(rain[0], iColumn);
    }
    else
    {
      //print("0");
      bitClear(rain[0], iColumn);
    }
  }
  //println();
}

// for each frame I just copy the frame above onto it
void advanceDrops()
{
  //print("advance ");println();
  // I need to copy starting at the bottom because 
  // otherwise I end up writing over myself
  for(byte iRow = rowCount-1;iRow > 0;iRow--)
  {
    rain[iRow] = rain[iRow-1];
  }
}

void animateRain()
{
  //print("animate");println();
  advanceDrops();
  seedDrop();
  //printRainState();

  for (byte iRow = 0;iRow < rowCount;iRow++)  
  {
    for(byte iColumn = 0;iColumn < 8;iColumn++)
    {
      byte currentPixel;
      // if it's an odd column we're dripping down
      if (iColumn % 2 == 1)
      {
        currentPixel = iRow +  iColumn * rowCount;
        if (bitRead(rain[iRow], iColumn))
        {
          leds[currentPixel] = CRGB(255,255,255);
        }
        else
          leds[currentPixel] = CRGB(0,0,0);
        //print("[");print(iRow);print(".");print(iColumn);print("->");print(iRow * 8 +  iColumn);println();
      }
      else // we're dripping up
      {
        currentPixel = iRow + iColumn * rowCount;
        if (bitRead(rain[rowCount-1-iRow], iColumn))
          leds[currentPixel] = CRGB(255,255,255);
        else
          leds[currentPixel] = CRGB(0,0,0);
      }
    }
  }
  FastLED.show();
  delay(50);
}

/*
void printRainState()
{
  for(int iRow = 0;iRow<rowCount;iRow++)
  {
    for(int iColumn = 0;iColumn < 8;iColumn++)
    {
      Serial.print(bitRead(rain[iRow],iColumn));
    }
    println();
  }
  Serial.println();
}
*/

void calm(bool isDay)
{
  static int frameCount = 0;
  if (frameCount == 0)
  {
    if (isDay)
      fill_solid(leds, ledCount, CHSV(40, 1, 128)); // daystorm
    else
      fill_solid(leds, ledCount, CHSV(244, 255, 29)); // daystorm
    FastLED.show();
  }
  
  if (frameCount == 512)
  {
    if (isDay)
      currentMode = modeSunset;
    else
      currentMode = modeSunrise;
    frameCount = 0;
  }
  delay(15);
  frameCount++;
}

void storm()
{
  //println(("in storm"));

  static int stormCount = 0;
  static bool inLightning = false;
  
  if (stormCount == 10)
  {
    // rainbows only happen in the day
    if (currentMode == modeNightStorm)
      currentMode = modeNight;
    else
      currentMode = modeRainbow;
    stormCount=0;
  }
  
  if (currentMode == modeDayStorm)
    fill_solid(leds, ledCount, CHSV(40, 1, 128)); // daystorm
  else
    fill_solid(leds, ledCount, CHSV(244, 255, 29)); // daystorm

  FastLED.show();
  
  if (random(500) == 0)
  {
    inLightning = true;
    stormCount++;
  }
  
  if (inLightning)
  {
    for(byte i = 0;i<128;i++)
    {
      flicker(240, 128);
      FastLED.show();
    }
    inLightning = false;
  }
}

void checkForEvent()
{
  //print(btnRainbow);print(" ");print(digitalRead(btnRainbow));print(" ");print(digitalRead(btnStorm));println();
  
  if (digitalRead(btnRainbow) == HIGH)
  {
    //println("btn high in modeRainbow");
    currentMode = modeRainbow;
  } 
  if (digitalRead(btnChangeDay) == HIGH)
  {
    //println("btn high in modeStorm");
    switch(currentMode)
    {
      case modeRainbow:
      case modeDayStorm:
        currentMode = modeSunset;
        break;
      case modeNightStorm:
        currentMode = modeSunrise;
        break;
      default:
        break;
    }      
  }  
}

void sunset()
{
  int hue = 45;
  //print("sunset ");print(hue);println();
  const byte delayTime = 30;
  for(int i = 0;i<=255;i++)
  {
    if (i % 5 == 0)
      hue = sunsetHue(hue);
    //print("HSVt ");print(hue);print(" ");print(i);print(" ");print(128);println();

    fill_solid(leds, ledCount, CHSV(hue, i, 128)); // daystorm
    FastLED.show();
    delay(delayTime);
  }
  for(byte i = 128;i>=20;i--)
  {
    if (i % 1 == 0)
      hue = sunsetHue(hue);
    //Serial.print("HSVt ");Serial.print(hue);Serial.print(" ");Serial.print(255);Serial.print(" ");Serial.print(i);Serial.println();
    fill_solid(leds, ledCount, CHSV(hue, 255, i)); // daystorm
    FastLED.show();
    delay(delayTime);
  }
  
  currentMode = modeNightStorm;
}

void sunrise()
{
  int hue = 45;
  //print("begin sunrise ");print(hue);println();
  const byte delayTime = 30;
  for(byte i = 20;i<=128;i++)
  {
    if (i % 1 == 0)
      hue = sunriseHue(hue);
    //Serial.print("HSVt ");Serial.print(hue);Serial.print(" ");Serial.print(255);Serial.print(" ");Serial.print(i);Serial.println();
    fill_solid(leds, ledCount, CHSV(hue, 255, i)); // daystorm
    FastLED.show();
    delay(delayTime);
  }
  for(byte i = 255;i>=0;i--)
  {
    if (i % 5 == 0)
      hue = sunriseHue(hue);
    //Serial.print("HSVt ");Serial.print(hue);Serial.print(" ");Serial.print(i);Serial.print(" ");Serial.print(128);Serial.println();

    fill_solid(leds, ledCount, CHSV(hue, i, 128)); // daystorm
    FastLED.show();
    delay(delayTime);
  }
  //print("end sunrise ");print(hue);println();
  
  currentMode = modeDayStorm;
}


int sunsetHue(int myHue)
{
  if (myHue == 0)
  {
    myHue = 360;
  }
  
  if (myHue == 240)
  {
    myHue = 45;
  }
  myHue--;
  return myHue;
}

int sunriseHue(int myHue)
{
  if (myHue == 360)
  {
    myHue = 0;
  }
  
  if (myHue == 45)
  {
    myHue = 240;
  }
  myHue++;
  return myHue;
}

void flicker(int thishue, int thissat) {            //-m9-FLICKER EFFECT
  int random_bright = random(0,255);
  int random_delay = random(10,100);
  int random_bool = random(0,random_bright);
  if (random_bool < 10) {
    delay(random_delay);
    for(byte i = 0 ; i < ledCount; i++ ) {
      leds[i] = CHSV(thishue, thissat, random_bright); 
    }
  }
}

