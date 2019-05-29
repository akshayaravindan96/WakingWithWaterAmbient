//----------------------------------------------------
//      ME216M Final Project
//      Waking with Water
//      Tieler Callazo
//      Zinny Weli
//      Akshay Aravindan
//----------------------------------------------------

#include <SoftwareSerial.h>
#include <EventManager.h>           // event manager system
#include <Chrono.h>                       // timers
#include <Wire.h>                             // allows communication with i2c/twi devices
#include "config.h"                             // connectivity settings
#include <ESP8266WiFiMulti.h>       // wifi router
#include <ESP8266HTTPClient.h>      // wifi router stuff
#include <ArduinoJson.h>                 // json stuff
#include "Adafruit_VEML7700.h"        // light sensor
#include "Adafruit_NeoPixel.h"              // led strips
#include <SPI.h>                                  // used for the screen
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h> // Hardware-specific library
#include <Adafruit_STMPE610.h> // resistive touchscreen controller

//----------------------------------------------------
//     Pinouts
//----------------------------------------------------

// TFT Touchscreen
const int STMPE_CS = 16;
const int TFT_CS = -1; // free up this pin; originally pin 0
const int TFT_DC = 15;
const int SD_CS = -1; // free up this pin; originally pin 2

const int PIN_AMBIENT_LED = 0; // update these later
const int PIN_DOCK_LED = 2;
const int PIN_MP3_TX = 1;
const int PIN_MP3_RX = 3; 

//----------------------------------------------------
//      Object Creation
//----------------------------------------------------
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT); // declaraion of SSD1306 on i2c (SDA, SCL)
// Create a display object similar to above
Chrono timerRefresh;                 // timer for updating time
Chrono timerAlarmTimeout;       // timer for checking if the alarm should shut off automatically
ESP8266WiFiMulti myWiFi;      // create wifi object
Adafruit_VEML7700 veml = Adafruit_VEML7700(); // light sensor

// creating the screen objects
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC); // tft screen
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS); // touchscreen controller
// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750
// Size of the color selection boxes and the paintbrush size
#define BOXSIZE 40
#define PENRADIUS 3
int oldcolor, currentcolor;
// -----------potentially delete: boxsize, penradius, oldcolor, currentcolor

//----------------------------------------------------
//      State Tracking Variables
//----------------------------------------------------

//----------------------------------------------------
//     Event & State Variables
//----------------------------------------------------
EventManager eventManager;
#define EVENT_TIMER_REFRESH EventManager::kEventUser0
#define EVENT_12AM EventManager::kEventUser1
#define EVENT_TAP_SCREEN EventManager::kEventUser2
#define EVENT_ALARM_TIMEOUT EventManager::kEventUser3
#define EVENT_ALARM_TIME EventManager::kEventUser4
#define EVENT_LIGHT_LEVEL EventManager::kEventUser5

//----------------------------------------------------
//     States for SM
//----------------------------------------------------
enum SystemState_t {STATE_INIT, STATE_TIME, STATE_ALARM};
SystemState_t currentState = STATE_INIT;

//----------------------------------------------------
//     Global Variables
//----------------------------------------------------
const int maxLux = 475;               // daylight lux
const int minLux = 10;                  // nighttime lux
const int NUM_PIXELS = 22;          // esimtated 22 pixels 

const int SCREEN_HEIGHT = 320;
const int SCREEN_WIDTH = 240;

Adafruit_NeoPixel strip(NUM_PIXELS, PIN_AMBIENT_LED, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  Serial.println("SETUP");
  InitVEML();                                   // init light sensor
  InitDisplay();                                // init TFT display
//  InitWiFi();                                     // wifi initialization 

  // consider making the strip its own init function
  InitNeoStrip();

  // initialize listeners
  eventManager.addListener(EVENT_TIMER_REFRESH, alarm_SM);
  eventManager.addListener(EVENT_12AM, alarm_SM);
  eventManager.addListener(EVENT_TAP_SCREEN, alarm_SM);
  eventManager.addListener(EVENT_ALARM_TIMEOUT, alarm_SM);
  eventManager.addListener(EVENT_ALARM_TIME, alarm_SM);
  eventManager.addListener(EVENT_LIGHT_LEVEL, alarm_SM);
  alarm_SM(STATE_TIME, 0); //initialize the state machine
  
  // restart the timers
  timerRefresh.restart();
  timerAlarmTimeout.restart();
}

void InitNeoStrip() {
  strip.begin();
  strip.show();
}

void InitVEML() {
    if (!veml.begin()) {
    Serial.println("VEML not found");
    while (1);
  }
  Serial.println("VEML found");
  veml.setGain(VEML7700_GAIN_1);
  veml.setIntegrationTime(VEML7700_IT_800MS);
}

void InitDisplay() {
  // initialization stuff
  if(!ts.begin()) {
    Serial.println("Touchscreen not found");
    while(1);
  }
  Serial.println("Touchscreen found");

  tft.begin();
  tft.setRotation(2); // sets the rotation
  tft.fillScreen(ILI9341_BLACK);
}

void InitWiFi() {
  WiFi.mode(WIFI_STA);
  myWiFi.addAP(wifi_ssid, wifi_password);
  delay(100);
  Serial.print("[WIFI] Initializing");
  for (int i = 0; i < 10; i++) {
    // wait for 10 seconds...
    // (hopefully enough time for WiFi to connect)
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println("[WIFI] Set up complete!");

  // prototyping
//  testScreen();
}

void loop() {
  // handle events in the queue
//  eventManager.processEvent();

  // check for new events
//  CheckRefreshTimer();
//  Check12AM();
//  CheckAlarmTime();
//  CheckScreenTap();
//  CheckAlarmTimerExpired();
  
  // prototyping functions
  CheckLux();
  testScreenLux();
  delay(3000);
}

void testScreen() {
  tft.fillScreen(ILI9341_BLUE);
  // time in the middle
  int x_1 = 20;
  int y_time = SCREEN_HEIGHT/2 - 20;
  tft.setCursor(x_1, y_time); // cursor location for time
  tft.setTextColor(ILI9341_WHITE); 
  tft.setTextSize(5);
  tft.println("6:00 AM");
}

void testScreenLux() {
  tft.fillScreen(ILI9341_BLACK);
  int x_1 = 20;
  int y_time = SCREEN_HEIGHT/2 - 20;
  tft.setCursor(x_1, y_time); // cursor location for time
  tft.setTextColor(ILI9341_WHITE); 
  tft.setTextSize(3);
  tft.print("LUX: "); tft.println(veml.readLux());
}

void CheckLux() {
  int currentLux = veml.readLux(); // 1) create  int for current lux
  static int prevLux = currentLux;  // 2) create static int for previous lux

  // 3) check if the light level has changed
  if(currentLux >= (prevLux + 20) || currentLux <= (prevLux - 20) ) {
    Serial.print("\nLight Level Changed from: "); Serial.print(prevLux); 
    Serial.print(" to: "); Serial.println(currentLux);
    prevLux = currentLux;
    eventManager.queueEvent(EVENT_LIGHT_LEVEL, 0);
  }
}

void RespLuxChange() {
  // colors for the different days
  uint32_t dawn_color = strip.Color(255, 224, 153);
  uint32_t day_color = strip.Color(255, 229, 204);
  uint32_t dusk_color = strip.Color(126, 125, 191);
  uint32_t night_color = strip.Color(115, 100, 242);
  
  // 1) check the time of day and determine the ratio of RGB
  strip.fill(dawn_color, 0, NUM_PIXELS - 1);
  
  // 2) scale the brightness based on the ambient light
  int intensity = map(veml.readLux(), minLux, maxLux, 26, 255);
  strip.setBrightness(intensity); // range from 0 off  to 255 full brightness
  strip.show();
  
  // 3) *optional* make the adustment more gradual 
}

void CheckRefreshTimer() {
  if (timerRefresh.hasPassed(60000)) {
    timerRefresh.restart();
    eventManager.queueEvent(EVENT_TIMER_REFRESH, 0);
  }
}

void Check12AM() {
  // if the current time is midnight
}

void CheckAlarmTime() {
  // if current time is the alarm time queue up the event
}

void CheckScreenTap() {
  // check if the screen has been tapped
}

void CheckAlarmTimerExpired() {
  if (timerAlarmTimeout.hasPassed(300000)) {
    eventManager.queueEvent(EVENT_ALARM_TIMEOUT, 0);
  }
}

void alarm_SM(int event, int param) {
  SystemState_t nextState = currentState;

  // initlialize the sunrise and sunset times
  static time_t sunriseTime = 0;
  static time_t sunsetTime = 0;
  static time_t currentTime = 0;

  // maybe create an init state where we update subriseTime and sunsetTime
  // and the ambient light levels before heading into the time mode

  switch(currentState) {
    case STATE_INIT: 
      // run the setup then transition to time
      // 1) pull sunrise and sunset from darksky
      // 2) update the lights to the right level
//      UpdateAmbient(currentTime, sunriseTime, sunsetTime);
      nextState = STATE_TIME;
      break;
      
    case STATE_TIME:
      if ( event == EVENT_LIGHT_LEVEL ) {
        Serial.println("Light level adjustment");
        RespLuxChange();
      }
      
      if (event == EVENT_TIMER_REFRESH) {
        UpdateDisplayTime();
//        UpdateHueColor();
      }

      if(event == EVENT_12AM) {
        // update the sunrise and sunset times
      }

      if(event== EVENT_ALARM_TIME) {
        // transition to alarm state
        // 1) play sound of water
        // 2) illuminate dock light
        // 3) restart alarm timout
        timerAlarmTimeout.restart();
      }
      break;

    case STATE_ALARM:
      if (event == EVENT_LIGHT_LEVEL) {
        Serial.println("Light level adjustment");
        RespLuxChange();
      }
      break;

    default:
      Serial.println("STATE: unknown.\tCheck State Transitions");
  }
}

void UpdateDisplayTime() {
  // dispaly the current time
}


//void UpdateAmbient(int currentTime, int timeSunrise, int timeSunset) {
//  // take the current time and check if what the color should be
//  // if within 1hr of sunrise or sunset, display the transition
//
//  // 1) measure ambient light for brightness
//  int lux = veml.readLux();
//  int maxPWM = map(lux, minLux, maxLux, 0, 1023); // map from 0% duty cycle to 100%
//  
//  // 2) check what time it is for the color of LEDs
//  if(currentTime < timeSunrise - 30) { // should be night time
//    // night: 115, 100, 242
//    UpdateRGB (115, 100, 242, maxPWM);
//  } else if (currentTime < timeSunrise + 30) {
//    // dawn: 255, 224, 153
//    UpdateRGB (255, 224, 153, maxPWM);
//  } else if (currentTime >= timeSunrise + 30 && currentTime < timeSunset - 30) {
//    // day: 255, 229, 204
//    UpdateRGB (255, 229, 204, maxPWM);
//  } else if (currentTime >= timeSunrise + 30 && currentTime < timeSunset + 30) {
//    // dusk: 126, 125, 191
//    UpdateRGB (126, 125, 191, maxPWM);
//  } else {
//    // night: 115, 100, 242
//    UpdateRGB (115, 100, 242, maxPWM);
//  }
//}

//void UpdateRGB (int R_CODE, int G_CODE, int B_CODE, int PWM_MAX) {
//  analogWrite(PIN_R_LED, R_CODE * PWM_MAX / 255);
//  analogWrite(PIN_G_LED, G_CODE * PWM_MAX / 255);
//  analogWrite(PIN_B_LED, B_CODE * PWM_MAX / 255);
//}
