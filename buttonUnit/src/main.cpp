#include <Arduino.h>
#include "../lib/main.h"

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#define UNIT_NAME "ButtonUnit"
#define ID 0
#define BUTTON_PIN D2
#define LED_PIN D4
#define UNIT_PORT 43432

String serverIP = "";
String serverPort = "";

bool status;
bool lastStatus;
bool serverShouldRun = true;

String output;

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
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

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
  status = digitalRead(BUTTON_PIN);
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

void setLed() {
  digitalWrite(LED_PIN, (status == 1 ? HIGH : LOW));
}

void loop() {

  if(serverShouldRun)
    server.handleClient();

  if(serverIP == "" || serverPort == "") {
    return;
  }

  readSensor();

  if(status != lastStatus) {
    output = String("ID=") + ID + String("&value=") + String(status);
    sendString(output);
    lastStatus = status;
  }

  setLed();

  delay(200);
}
