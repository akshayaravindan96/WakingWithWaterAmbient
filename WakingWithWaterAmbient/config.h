
/******************************* WIFI **************************************/

// WiFi credentials for use with ESP8266WiFi/ESP8266WiFiMulti library

const char* wifi_ssid = "ME216M Wi-Fi Network";  // network name
const char* wifi_password = "me216marduino";     // network password

/************************ Adafruit IO Config *******************************/

// visit io.adafruit.com if you need to create an account,
// or if you need your Adafruit IO key.
const char* io_key = "d07dc3c9224847e0a1debaef6d040826";  // YOUR ADAFRUIT IO KEY

const String io_host = "io.adafruit.com";
const String io_username = "tieler";
const String io_feedkey = "sensor-data";
const String io_feed_path = io_host + "/api/v2/" + io_username + "/feeds/" + io_feedkey + "/data";

// SHA1 fingerprint certificate for io.adafruit.com (do not change)
const char* io_fingerprint = "77 00 54 2D DA E7 D8 03 27 31 23 99 EB 27 DB CB A5 4C 57 18";


/******************** Advice Slip API Config ***************************/
// Uses Advice Slip API to generate random advice.

// In accordance with the Advice Slip API. See https://api.adviceslip.com/ for details.
const String ad_path = "api.darksky.net/forecast/210a11b14f87ad6057573179b0522368/37.4241,-122.1660?exclude=minutely,hourly,alerts,flags";  // URL for random advice (excluding https://)

// SHA1 fingerprint certificate for adviceslip.com (do not change)
const char* ad_fingerprint = "EA C3 0B 36 E8 23 4D 15 12 31 9B CE 08 51 27 AE C1 1D 67 2B";
