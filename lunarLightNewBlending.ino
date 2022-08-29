/*  Simple test for ebay 128x64 Serial run via I2C on a 
    Arduino Pro Mini 3.3V (328P), with ebay DS3231 real time
    clock module.
    
    The sketch will calculate the current tide height for
    the site (assuming clock is set correctly) and display
    the tide height and time on a ssd1306-controller Serial
    128x64 display. Time updates every second, tide updates
    as the last significant digit changes (10-20 seconds). 

*/

#include <Wire.h> 
#include <FastLED.h>
#include <TFT_eSPI.h> 
#define LED_PIN     26
#define NUM_LEDS    36
#define BRIGHTNESS  255
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];
CRGB ledsBackground[55];
int section;
float timer = 0;
#include <SPI.h> // Required for RTClib to compile properly
#include <RTClib.h> // From https://github.com/millerlp/RTClib
// Real Time Clock setup
RTC_DS3231 RTC; // Uncomment when using this chip
int dS = 0;
DateTime futureHigh;
DateTime futureLow;
DateTime future;
int slope;
int i = 0;
int zag = 0;
bool gate = 1;
float tidalDifference=0;
 float pastResult;
 bool mainHiLow = 1;
 bool bing = 1;
 bool futureLowGate = 0;
 bool futureHighGate = 0;
 int hourDifference = 0;
CRGBPalette16 currentPalette;
CRGBPalette16 currentPalette2;
static uint8_t startIndex = 0;
TBlendType    currentBlending;
TFT_eSPI tft = TFT_eSPI();
// Tide calculation library setup.
// Change the library name here to predict for a different site.
//#include "TidelibSanDiegoSanDiegoBay.h"
#include "TidelibValdezPrinceWilliamSoundAlaska.h"
// Other sites available at http://github.com/millerlp/Tide_calculator
TideCalc myTideCalc; // Create TideCalc object 

// 0X3C+SA0 - 0x3C or 0x3D for Serial screen on I2C bus




long oldmillis; // keep track of time
float results; // tide height
DateTime now; // define variable to hold date and time
// Enter the site name for display. 11 characters max
char siteName[11] = "SITKA";  
//------------------------------------------------------------------------------

void graphTide(DateTime now, DateTime futureHigh, DateTime futureLow,int dS){
  
  
  
  
   if(futureHigh.unixtime() > futureLow.unixtime() ){
      hourDifference =  ((futureLow.unixtime() - now.unixtime())/3600);
     mainHiLow = 0;
   }
   else {
     hourDifference =  ((futureHigh.unixtime() - now.unixtime())/3600);
    mainHiLow = 1;
   }
   
   tft.setCursor(10, 80);
   tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.print(hourDifference);
   tft.print(" HOURS TO");
   if(mainHiLow)tft.println("   HIGH"); 
   else tft.println("    LOW");
   
   
    Serial.println();
    hourDifference = map(hourDifference, 0,6,16,0);
    hourDifference = constrain(hourDifference,0,16);
    tft.setCursor(60, 105);
    tft.println(myTideCalc.currentTide(now));
  
       
}

void SerialScreen(DateTime now, DateTime futureHigh, DateTime futureLow,int dS){
   bool hiLow;
   tft.fillScreen(TFT_BLACK);
   tft.setTextColor(TFT_YELLOW, TFT_BLACK);
   tft.setTextFont(4);
   tft.setCursor(0, 0);
   if( int(futureHigh.unixtime() - futureLow.unixtime()) < 0) hiLow = 1;
   if( int(futureHigh.unixtime() - futureLow.unixtime()) > 0) hiLow = 0;
 
  if (hiLow) {
    tft.print("HI         ");
    tft.println("LOW");
    tft.print(futureHigh.hour() +1, DEC); 
  tft.print(":");
  if (futureHigh.minute() < 10) {
    tft.print("0");
    tft.print(futureHigh.minute());
   }
  else if (futureHigh.minute() >= 10) {
    tft.print(futureHigh.minute());
  }
  tft.print("       ");
  tft.print(futureLow.hour() +1, DEC); 
  tft.print(":");
  if (futureLow.minute() < 10) {
    tft.print("0");
    tft.print(futureLow.minute());
   }
  else if (futureLow.minute() >= 10) {
   tft.print(futureLow.minute());
  }
    
  }
  else {
    tft.print("LOW         ");
    tft.println("HI");
    tft.print(futureLow.hour() +1, DEC); 
  tft.print(":");
  if (futureLow.minute() < 10) {
   tft.print("0");
   tft.print(futureLow.minute());
   }
  else if (futureLow.minute() >= 10) {
    tft.print(futureLow.minute());
  }
  Serial.print("       ");
   tft.print(futureHigh.hour() +1, DEC); 
   tft.print(":");
  if (futureHigh.minute() < 10) {
    tft.print("0");
    tft.print(futureHigh.minute());
   }
  else if (futureHigh.minute() >= 10) {
    tft.print(futureHigh.minute());
  }
  }
  tft.println();
  tft.println();
  tft.println();
  tft.println();
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.drawCentreString("Whittier Dock Tide", 125, 55, 4);
 
results = myTideCalc.currentTide(now); 
      
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC); 
  Serial.print("/");
  Serial.print(now.day(), DEC); 
  Serial.print("  ");
  Serial.print(now.hour() + 1, DEC); 
  Serial.print(":");
  if (now.minute() < 10) {
    Serial.print("0");
    Serial.println(now.minute());
   }
  else if (now.minute() >= 10) {
    Serial.println(now.minute());
  }
 Serial.print(" ");

}
void setup() {
  Wire.begin();  
  RTC.begin();
  Serial.begin(57600);
  //RTC.adjust(DateTime(F(__DATE__), F(__TIME__))); 
  // Start up the Serial display
  FastLED.addLeds<LED_TYPE, 26, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.addLeds<LED_TYPE, 13, COLOR_ORDER>(ledsBackground, 55);
  FastLED.setBrightness(  BRIGHTNESS );
     //fill_solid( currentPalette2, 16, CRGB::White);
    currentPalette = LavaColors_p;
    currentPalette2 = OceanColors_p;
    //fill_solid( currentPalette2, 16, CRGB::White);
//fill_solid( currentPalette, 16, CRGB::Red);
    currentBlending = LINEARBLEND;
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextFont(4);
   
     fill_solid(ledsBackground, 55, CRGB::Yellow);
      FastLED.show();
}
//------------------------------------------------------------------------------
void loop() {
  // The main statement block will run once per second
  EVERY_N_MINUTES(1){    
      now = RTC.now(); // update time
      
      // Calculate current tide height
       if((now.month()>3&&now.month()<11)||(now.month()==3&&now.day()>11)||(now.month()==11&&now.day()<6))dS=1;
   now = (now.unixtime() - dS*3600);
   Serial.print(now.hour());
   if(now.hour() >= 21 || now.hour() < 6){    
    fill_solid(ledsBackground, 55, CRGB::Blue);
   }else if(now.hour() >= 6 && now.hour() < 8 ){
   fill_solid(ledsBackground, 55, CRGB::Yellow);
   }
   else {
    fill_solid(ledsBackground, 55, CRGB::White);
   }
    FastLED.show();
    
   bing = 1;
   i = 0;
    pastResult=myTideCalc.currentTide(now);
  while(bing){ //This loop asks when the next high or low tide is by checking 15 min intervals from current time
    i++;
   
    DateTime future(now.unixtime() + (i*5*60L));
    results=myTideCalc.currentTide(future);
    tidalDifference=results-pastResult;
    if (gate){
      if(tidalDifference<0)slope=0;//if slope is positive--rising tide--slope neg falling tide
      else slope=1;
      gate=0;
   }
   if(tidalDifference>0&&slope==0){
      futureLow = future;
      gate=1;
      //bing = 0;
      futureLowGate = 1;
   }
    else if(tidalDifference<0&&slope==1){
    futureHigh = future;
    gate=1;
    //bing = 0;
    futureHighGate = 1;
  
   }
   if( futureHighGate && futureLowGate) {
  
    SerialScreen(now, futureHigh, futureLow,dS);
    delay(1000);
    graphTide( now, futureHigh, futureLow, dS);
    
    delay(1000);
    gate = 1;
    bing = 0;
    futureHighGate = 0;
    futureLowGate = 0;
   }
    pastResult=results;
    //Serial.print("results");
    //Serial.print(results);
    //Serial.print(future.year());
    
  }
  
  }
  if (mainHiLow){
    fillnoise8();
  }
  else {
    fill_time();
  }
  LEDS.show();
}
void fillnoise8() {
 
  #define scale 30                                                          // Don't change this programmatically or everything shakes.
  
  uint8_t brightness = 255;
      for( int i = 0; i < NUM_LEDS; i ++){
       uint8_t index = inoise8(i*scale, millis()/10+i*scale); 
        leds[i] = ColorFromPalette( currentPalette2, index, brightness,LINEARBLEND );
         
      }
      
     for( int i = 0; i < hourDifference; i = i+1) {
      
      uint8_t index = inoise8(i*scale, millis()/10+i*scale); 
        leds[i] = ColorFromPalette( currentPalette, index, brightness, LINEARBLEND);
        
    }
    
     for( int i = 34; i > 34 -(hourDifference ); i = i - 1) {
      
        uint8_t index = inoise8(i*scale, millis()/10+i*scale); 
        leds[i] = ColorFromPalette( currentPalette, index, brightness, LINEARBLEND);
        
   }
    leds[16] =  CRGB::Green;
   
    // With that value, look up the 8 bit colour palette value and assign it to the current LED.
  }
  void fill_time(){
  #define scale 30
 
      uint8_t brightness = 255;
      for( int i = 0; i < NUM_LEDS; i ++){
        uint8_t index = inoise8(i*scale, millis()/10+i*scale); 
        leds[i] = ColorFromPalette( currentPalette,index, brightness, LINEARBLEND);
       
      }
     for( int i = 16; i > 16 - (hourDifference + 2); i=i-1) {
      uint8_t index = inoise8(i*scale, millis()/10+i*scale); 
        leds[i] = ColorFromPalette( currentPalette2, index, brightness, LINEARBLEND);
        
    }
     for( int i = 17; i < 17 + hourDifference; i = i + 1) {
      uint8_t index = inoise8(i*scale, millis()/10+i*scale); 
        leds[i] = ColorFromPalette( currentPalette2, index, brightness, LINEARBLEND);
        
    }
    leds[34] = CRGB::Green;
}
