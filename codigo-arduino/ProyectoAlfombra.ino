#include "pitches.h"
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include "time.h"
#include "sntp.h"
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebSrv.h>


const char* ssid       = "Paula";
const char* password   = "12345678";

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = -10800;
const int   daylightOffset_sec = 0;

const char* time_zone = "ART3";

struct tm timeinfo;

// set pin numbers
const int touchPin1 = 4;
const int touchPin2 = 15;
const int ledPin1 = 17;
const int ledPin2 = 16;


LiquidCrystal lcd(19, 23, 12, 5, 18, 22);

// change with your threshold value
const int threshold = 40;
// variable for storing the touch pin value
int touchValue1;
int touchValue2;

AlarmId id;

AsyncWebServer server(80);
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}


// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

void setLocalTime()
{
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
  }
  else {
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    int hourNow = int (timeinfo.tm_hour);
    int minuteNow = int (timeinfo.tm_min);
    int secondNow = int (timeinfo.tm_sec);
    int dayNow = int (timeinfo.tm_mday);
    int monNow = int (timeinfo.tm_mon) + 1;
    int yearNow = int (timeinfo.tm_year) - 100;
    setTime(hourNow, minuteNow, secondNow, dayNow, monNow, yearNow);
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(String(hourNow) + ":" + String(minuteNow) + ":" + String(secondNow) + " " + String(dayNow) + "/" + String(monNow) + "/" + String(yearNow));

  }
}

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
  }
  else {
      setLocalTime();
  }
}

bool sensor_toque(int touchPin, int ledPin) {
  int touchValue = touchRead(touchPin);
  if (touchValue < threshold) {
    // turn LED on
    digitalWrite(ledPin, HIGH);
    Serial.print(touchValue);
    Serial.println(" - LED on");
    return true;
  }
  else {
    // turn LED off
    digitalWrite(ledPin, LOW);
    Serial.println(" - LED off");
    return false;
  }
}

void setup() {
  lcd.begin(16, 2);
  pinMode (ledPin1, OUTPUT);
  pinMode (ledPin2, OUTPUT);
  Serial.begin(9600);
  while (!Serial) ; // wait for Arduino Serial Monitor
    sntp_set_time_sync_notification_cb( timeavailable );
  sntp_servermode_dhcp(1);    // (optional)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");

  Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    lcd.setCursor(0,1);
    lcd.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Servidor en ESP32");
    });

    server.on("/crearalarma", HTTP_POST, [](AsyncWebServerRequest *request){
        String alarm_name;
        int hora;
        int minuto;
        int repetir;
        if (request->hasParam("name", true)) {
            alarm_name = request->getParam("name", true)->value();
        }
        if (request->hasParam("hour", true)) {
            String hor = request->getParam("hour", true)->value();
            hora = hor.toInt();
        }
        if (request->hasParam("minute", true)) {
            String minut = request->getParam("minute", true)->value();
            minuto = minut.toInt();
        }
        if (request->hasParam("repeat", true)) {
            String rep = request->getParam("repeat", true)->value();
            repetir = rep.toInt();
        }

        if (repetir == 1) {
          Alarm.alarmOnce(hora, minuto, 0, playMelody);
        }
        else if (repetir == 2) {
          Alarm.alarmRepeat(hora, minuto, 0, playMelody);
        }
        request->send(200, "text/plain", "Alarma creada");
    });

    server.onNotFound(notFound);
    server.begin();
}

void loop() {
  setLocalTime();
    sensor_toque(touchPin1, ledPin1);
  sensor_toque(touchPin2, ledPin2);
  Alarm.delay(600);
}

void playMelody() {
  while (!(sensor_toque(touchPin1, ledPin1)) && !(sensor_toque(touchPin2, ledPin2))) {
    for (int thisNote = 0; thisNote < 8; thisNote++) {
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(2, melody[thisNote], noteDuration);
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(2);
    if (sensor_toque(touchPin1, ledPin1) && sensor_toque(touchPin2, ledPin2)) {
      break;
    }
  }
  }
  }
