#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

const char *ssid = "";
const char *password = "";

const char *serviceName = "fan";

ESP8266WebServer server(80);

String rootHtml;

const int led = 13;

enum FunctionPin {
  light = D1,
  high = D2,
  medium = D3,
  low = D4,
  off = D5,
  reverse = D6,
};

void readRootHtml() {
  SPIFFS.begin();

  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    Serial.println("file open failed");
  } else {
    Serial.println("file open success");

    rootHtml = "";
    while (file.available()) {
      //Serial.write(file.read());
      String line = file.readStringUntil('\n');
      rootHtml += line + "\n";
    }
    file.close();
  }
}

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/html", rootHtml);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void handleFunction(FunctionPin pin) {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "OK");
  digitalWrite(led, 0);

  digitalWrite(pin, 1);
  delay(500);
  digitalWrite(pin, 0);
}

void setup(void) {
  int pins[] = { led, light, high, medium, low, off, reverse };
  for (auto pin : pins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, 0);
  }
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(serviceName)) {
    Serial.println("MDNS responder started");
  }

  readRootHtml();

  server.on("/", handleRoot);

  server.on("/function/light", []() {
    handleFunction(light);
  });
  server.on("/function/high", []() {
    handleFunction(high);
  });
  server.on("/function/medium", []() {
    handleFunction(medium);
  });
  server.on("/function/low", []() {
    handleFunction(low);
  });
  server.on("/function/off", []() {
    handleFunction(off);
  });
  server.on("/function/reverse", []() {
    handleFunction(reverse);
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
