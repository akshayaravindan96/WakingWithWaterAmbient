#include <Chrono.h>
#include <LightChrono.h>

//----------------------------------------------------
//      ME216M Final Project
//      Waking with Water
//      Tieler Callazo
//      Zinny Weli
//      Akshay Aravindan
//----------------------------------------------------

#include <EventManager.h>           // event manager system
                      // timers
#include <Wire.h>                             // allows communication with i2c/twi devices
#include "config.h"                             // connectivity settings
#include <ESP8266WiFiMulti.h>       // wifi router
#include <ESP8266HTTPClient.h>      // wifi router stuff
          
#include "Adafruit_VEML7700.h"        // light sensor
#include "Adafruit_NeoPixel.h"              // led strips

#include <ArduinoJson.h>

//----------------------------------------------------
//     Pinouts
//----------------------------------------------------


const int PIN_AMBIENT_LED = 12; // update these later


//----------------------------------------------------
//      Object Creation
//----------------------------------------------------
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT); // declaraion of SSD1306 on i2c (SDA, SCL)
// Create a display object similar to above
Chrono timerRefresh;                 // timer for updating time
Chrono timerAlarmTimeout;       // timer for checking if the alarm should shut off automatically
ESP8266WiFiMulti myWiFi;      // create wifi object
Adafruit_VEML7700 veml = Adafruit_VEML7700(); // light sensor


//----------------------------------------------------
//      State Tracking Variables
//----------------------------------------------------

//----------------------------------------------------
//     Event & State Variables
//----------------------------------------------------
EventManager eventManager;
#define EVENT_TIMER_REFRESH EventManager::kEventUser0
#define EVENT_LIGHT_LEVEL EventManager::kEventUser5

//----------------------------------------------------
//     States for SM
//----------------------------------------------------
enum SystemState_t {STATE_MAIN};
SystemState_t currentState = STATE_MAIN;

//----------------------------------------------------
//     Global Variables
//----------------------------------------------------
const int maxLux = 475;               // daylight lux
const int minLux = 10;                  // nighttime lux
const int NUM_PIXELS = 30;          // esimtated 22 pixels 



Adafruit_NeoPixel strip(NUM_PIXELS, PIN_AMBIENT_LED, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  Serial.println("SETUP");
  InitVEML();                                   // init light sensor
                             
//  InitWiFi();                                     // wifi initialization 

  // consider making the strip its own init function
  InitNeoStrip();

  // initialize listeners
  eventManager.addListener(EVENT_TIMER_REFRESH, alarm_SM);
  eventManager.addListener(EVENT_LIGHT_LEVEL, alarm_SM);
  
  Ambient_SM(STATE_MAIN, 0); //initialize the state machine
  
  // restart the timers
  timerRefresh.restart();
  
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


}

void loop() {
  // handle events in the queue
 eventManager.processEvent();

  // check for new events
//  CheckRefreshTimer();
//  Check12AM();
//  CheckAlarmTime();
//  CheckScreenTap();
//  CheckAlarmTimerExpired();
  
  // prototyping functions
  CheckLux();

  delay(3000);
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


void Ambient_SM(int event, int param) {
  SystemState_t nextState = currentState;

  // initlialize the sunrise and sunset times
  static time_t sunriseTime = 0;
  static time_t sunsetTime = 0;
  static time_t currentTime = 0;

  // maybe create an init state where we update subriseTime and sunsetTime
  // and the ambient light levels before heading into the time mode

  switch(currentState) {
      
    case STATE_MAIN:
      if ( event == EVENT_LIGHT_LEVEL ) {
        Serial.println("Light level adjustment");
        RespLuxChange();
      }
      if (event == EVENT_TIMER_REFRESH) {
        //UpdateDisplayTime();
//        UpdateHueColor();
      }

      break;

    default:
      Serial.println("STATE: unknown.\tCheck State Transitions");
  }
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
