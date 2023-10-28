//Incluir librerías
#include "pitches.h"
#include <TimeLib.h>
#include <TimeAlarms.h>
#include <LiquidCrystal.h>
#include <WiFi.h>
#include <WiFiMulti.h>
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

//Definir variables y constantes globales

//Configuración del servidor de la hora y timezone
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = -10800;
const int   daylightOffset_sec = 0;

const char* time_zone = "ART3";

struct tm timeinfo;

//Configurar pines táctiles y LEDs
const int touchPin1 = 4;
const int touchPin2 = 15;
const int ledPin1 = 17;
const int ledPin2 = 16;
const int threshold = 20;
int touchValue1;
int touchValue2;

//Configurar pines de LCD
LiquidCrystal lcd(19, 23, 26, 32, 18, 22);

WiFiMulti wifiMulti;
const uint32_t connectTimeoutMs = 15000;

AlarmId id;

//Crear servidor asincrónico
AsyncWebServer server(80);

//Definir función callback para el error 404
void notFound(AsyncWebServerRequest *request) {
		request->send(404, "text/plain", "Not found");
}


// Melodía para el sonido de la alarma
int melody[] = {
	NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// Duración de cada nota del sonido de la alarma
int noteDurations[] = {
	4, 8, 8, 4, 4, 4, 4, 4
};

//Definición de función que configura la hora del RTC y la imprime en la LCD
void setLocalTime()
{
	if(!getLocalTime(&timeinfo)){
		Serial.println("No time available (yet)");
	}
	else {
		int hourNow = int (timeinfo.tm_hour); //Número entero con la hora actual
		int minuteNow = int (timeinfo.tm_min); //Número entero con el minuto actual
		int secondNow = int (timeinfo.tm_sec); //Número entero con el segundo actual
		int dayNow = int (timeinfo.tm_mday); //Número entero con el número de fecha actual
		int monNow = int (timeinfo.tm_mon) + 1; //Número entero que representa el número de mes actual
		int yearNow = int (timeinfo.tm_year) - 100; //Número entero con los últimos dos dígitos del año
		setTime(hourNow, minuteNow, secondNow, dayNow, monNow, yearNow); //Configura la fecha y hora
	printLocalTime();
	}
}

void printLocalTime() {
	lcd.setCursor(0, 0);
	lcd.print(String(hour()) + ":" + String(minute()) + " " + String(day()) + "/" + String(month()) + "/" + String(year())); //Imprime fecha y hora en la LCD
	Serial.println(String(hour()) + ":" + String(minute()) + " " + String(day()) + "/" + String(month()) + "/" + String(year()));
}

// Definición de función callback para cuando el servidor NTP responde la solicitud y devuelve la hora
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

// Definición de función que evalúa si se está tocando el pin táctil, y en consecuencia prende o apaga un led
bool sensor_toque(int touchPin, int ledPin) {
	int touchValue = touchRead(touchPin);
	if (touchValue < threshold) {
		// turn LED on
	//Serial.println(touchValue);
		digitalWrite(ledPin, HIGH);
		return true;
	}
	else {
		// turn LED off
		digitalWrite(ledPin, LOW);
		return false;
	}
}

//Función setup, se llama al ejecutar el código
void setup() {
	lcd.begin(16, 2); //Inicializar pantalla LCD
	pinMode (ledPin1, OUTPUT); //Definir pines de led como salida
	pinMode (ledPin2, OUTPUT);
	Serial.begin(9600); //Inicializar puerto serial
	while (!Serial) ; // wait for Arduino Serial Monitor
		sntp_set_time_sync_notification_cb( timeavailable ); //Hacer solicitud al servidor de la hora
	sntp_servermode_dhcp(1);
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2); // Configurar hora y timezone

  //Configura una lista de SSIDs y contraseñas para conectarse
	wifiMulti.addAP("Paula", "12345678");
	wifiMulti.addAP("TeleCentro-acff", "JJZATZ4MMNJZ");
	wifiMulti.addAP("elthom", "wengchan7");

	//Conectarse a WiFi
	WiFi.mode(WIFI_STA);
	if(wifiMulti.run() == WL_CONNECTED) {
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
	lcd.setCursor(0,1);
	lcd.println(WiFi.localIP());
}

	//Al acceder a ruta "/" del servidor web
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
			request->send(200, "text/plain", "Servidor en ESP32");
	});

	//Al acceder a ruta "/crearalarma" del servidor web con método POST
	server.on("/crearalarma", HTTP_POST, [](AsyncWebServerRequest *request){
			String alarm_name; //Nombre de la alarma
			int hora; //Hora de la alarma
			int minuto; //Minuto de la alarma
			int repetir; //Indica si la alarma debe ser repetida o no
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
				// Si repetir es 1, la alarma no se repite
				Alarm.alarmOnce(hora, minuto, 0, playMelody);
			}
			else if (repetir == 2) {
				//Si repetir es 2, la alarma se repite todos los días a la misma hora
				Alarm.alarmRepeat(hora, minuto, 0, playMelody);
			}
			request->send(200, "text/plain", "Alarma creada"); //Devuelve código 200
	});

		server.onNotFound(notFound); //Si se accede a una ruta no encontrada, mandar error 404 y función callback
		server.begin();
}

//Función loop, se ejecuta todo el tiempo
void loop() {
	printLocalTime(); //Actualiza la hora en tiempo real todo el tiempo
	if (wifiMulti.run(connectTimeoutMs) == WL_CONNECTED) {
	Serial.println(WiFi.localIP());
	lcd.setCursor(0,1);
	lcd.println(WiFi.localIP());
	}
	else {
	Serial.println("WiFi not connected!");
	}
	sensor_toque(touchPin1, ledPin1); //Sensa los pines táctiles
	sensor_toque(touchPin2, ledPin2);
	Alarm.delay(200); //Esperar
}

//Función callback de la alarma, toca una melodía mientras que no se estén tocando ambos pines táctiles a la vez
void playMelody() {
	while (!(sensor_toque(touchPin1, ledPin1)) && !(sensor_toque(touchPin2, ledPin2))) {
		for (int thisNote = 0; thisNote < 8; thisNote++) {
		int noteDuration = 1000 / noteDurations[thisNote];
		tone(25, melody[thisNote], noteDuration);
		int pauseBetweenNotes = noteDuration * 1.30;
		delay(pauseBetweenNotes);
		noTone(25);
		if (sensor_toque(touchPin1, ledPin1) && sensor_toque(touchPin2, ledPin2)) {
			break;
		}
	}
	}
}
