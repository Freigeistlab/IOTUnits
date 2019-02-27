#include <Arduino.h>
#include "../lib/main.h"

#include "HX711.h"
#include <SPI.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#define UNIT_NAME "ScaleUnit"
#define ID 0
#define DOUT_PIN 5
#define CLK_PIN 4
#define LED_PIN D4
#define UNIT_PORT 43432

#define DEFAULT_SENSITIVITY 10
#define DEFAULT_THRESHOLD 1000
#define DEFAULT_MODE CONTINOUS

String serverIP = "";
String serverPort = "";

uint16_t sensitivity = DEFAULT_SENSITIVITY;
uint16_t threshold = DEFAULT_THRESHOLD;
uint8_t mode = DEFAULT_MODE;

int16_t valueRead;
int16_t lastValueRead;
bool status;
bool lastStatus;
bool serverShouldRun = true;

String output;

float calibration_factor = -7050;

HX711 scale(DOUT_PIN, CLK_PIN);
ESP8266WebServer server(UNIT_PORT);
WiFiManager wifiManager;

void handleRoot()
{
  Serial.println("handleRoot started");

  if(!server.hasArg("ip") || !server.hasArg("port")) {
    Serial.println("invalid request");
    server.send(404, "text/plain", "client did not provide ip or port");
    return;
  }
  serverIP = server.arg("ip");
  serverPort = server.arg("port");
  Serial.println(String("set server IP and PORT: ") + String(serverIP) + String(":") + String(serverPort));

  server.send(200, "text/plain", String(UNIT_NAME) + String(ID));
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleConfigure() {
  Serial.println("handleConfigure started");

  String message;

  if(server.hasArg("threshold")) {
    threshold = server.arg("threshold").toInt();
    message += String("set threshold to ") + String(threshold) + String(";");
  }

  if(server.hasArg("sensitivity")) {
    sensitivity = server.arg("sensitivity").toInt();
    message += String("set sensitivity to ") + String(sensitivity) + String(";");
  }
  
  if(server.hasArg("mode")) {
    mode = server.arg("mode").toInt();
    message += String("set mode to ") + String(mode) + String(";");
  }

  if(server.hasArg("end")) {
    message += String("will close server;");
    serverShouldRun = false;
  }

  Serial.println(message);
  server.send(200, "text/plain", message);

  if(!serverShouldRun) {
    server.close();
  }
}

void setup()
{
  Serial.begin(9600);

  scale.set_scale(calibration_factor);
  scale.tare();

  wifiManager.autoConnect(UNIT_NAME);

  Serial.println("registering handleRoot");
  server.on("/", handleRoot);

  Serial.println("registering handleConfigure");
  server.on("/configure", handleConfigure);

  Serial.println("registering handleNotFound");
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void readSensor() {
  valueRead = scale.get_units();
  status = (valueRead > threshold);
}

void sendString(String output) {

  HTTPClient http;

  String requestDestination = String("http://") + String(serverIP) + String(":") + String(serverPort) + String("/") + String(UNIT_NAME);
  Serial.println(String("destination: ") + String(requestDestination));

  http.begin(requestDestination);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded; charset=UTF-8");

  int httpCode = http.POST(output);
  String payload = http.getString();

  Serial.println(String("send: ") + String(output));
  Serial.println(String("httpCode: ") + String(httpCode));
  Serial.println(String("payload: ") + String(payload));

  http.end();
}

void setLed()
{
  if (valueRead <= threshold)
    digitalWrite(LED_PIN, HIGH);
  else
    digitalWrite(LED_PIN, LOW);
}

void loop() {

  if(serverShouldRun)
    server.handleClient();

  if(serverIP == "" || serverPort == "") {
    return;
  }

  readSensor();

  switch(mode) {
    case BOOLEAN:
      if(status != lastStatus) {
        output = String("ID=") + ID + String("&mode=") + String(mode) + String("&value=") + String(status);
        sendString(output);
        lastStatus = status;
      }
      break;

    case CONTINOUS:
      if(abs(valueRead - lastValueRead) > sensitivity) {
        output = String("ID=") + ID + String("&mode=") + String(mode) + String("&value=") + String(valueRead);
        sendString(output);
        lastValueRead = valueRead;
      }
      break;

    case IDLE:
      break;
    
    default:
      break;
  }

  setLed();

  delay(200);
}