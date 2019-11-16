/*
  Example application to show how to save crash information
  to ESP8266's flash using EspSaveCrash library
  Please check repository below for details

  Repository: https://github.com/krzychb/EspSaveCrash
  File: RemoteCrashCheck.ino
  Revision: 1.0.2
  Date: 17-Aug-2016
  Author: krzychb at gazeta.pl

  Copyright (c) 2016 Krzysztof Budzynski. All rights reserved.

  This application is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This application is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "EspSaveCrash.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// WiFi ssid and password defined in wifiConfig.h
// file is tracked with gitignore
// #include "wifiConfig.h"
const char* ssid = "********";
const char* password = "********";

EspSaveCrash SaveCrash;

ESP8266WebServer server(80);

// the buffer to put the Crash log to
char *_debugOutputBuffer;

// flag for setting up the server with WiFiMulti.addAP
uint8_t performOnce = 0;

void setup(void)
{
  // 2048 should be able to carry most of the debug stuff
  _debugOutputBuffer = (char *) calloc(2048, sizeof(char));

  Serial.begin(115200);
  Serial.println("\nRemoteCrashCheck.ino");

  SaveCrash.print();

  Serial.printf("Connecting to %s ", ssid);

  // WiFi.persistent(false);
  WiFi.mode(WIFI_STA);

  // remove if using WiFiMulti.addAP
  WiFi.begin(ssid, password);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // end of remove

  // non blocking WiFi login, will be added to onboard WiFi settings
  // WiFiMulti.addAP(ssid, password);
  // you have to wait until WiFi.status() == WL_CONNECTED
  // before starting MDNS and the webserver

  Serial.println(" connected");

  server.on("/", handleRoot);
  server.on("/log", handleLog);
  server.onNotFound(handleNotFound);

  Serial.printf("HTTP server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());

  // remove if using WiFiMulti.addAP
  server.begin();
  if (MDNS.begin("esp8266"))
  {
    Serial.println("MDNS responder started");
  }
  // end of remove

  Serial.println("\nPress a key + <enter>");
  Serial.println("0 : attempt to divide by zero");
  Serial.println("e : attempt to read through a pointer to no object");
  Serial.println("c : clear crash information");
  Serial.println("p : print crash information");
  Serial.println("b : store crash information to buffer and print buffer");
}


void loop(void)
{
  server.handleClient();
  MDNS.update();

  /*
  // uncomment if using WiFiMulti.addAP
  // if a WiFi connection is successfully established
  if (WiFi.status() == WL_CONNECTED)
  {
    if (performOnce == 0)
    {
      performOnce = 1;

      server.begin();
      if (MDNS.begin("esp8266"))
      {
        Serial.println("MDNS responder started");
      }
    }
  }
  // end of uncomment
  */

  // read the keyboard
  if (Serial.available() > 0)
  {
    char inChar = Serial.read();
    switch (inChar)
    {
      case '0':
        Serial.println("Attempting to divide by zero ...");
        int result, zero;
        zero = 0;
        result = 1 / zero;
        Serial.print("Result = ");
        Serial.println(result);
        break;
      case 'e':
        Serial.println("Attempting to read through a pointer to no object ...");
        int* nullPointer;
        nullPointer = NULL;
        // null pointer dereference - read
        // attempt to read a value through a null pointer
        Serial.print(*nullPointer);
        break;
      case 'c':
        SaveCrash.clear();
        Serial.println("Crash information cleared");
        break;
      case 'p':
        Serial.println("--- BEGIN of crash info ---");
        SaveCrash.print();
        Serial.println("--- END of crash info ---");
        break;
      case 'b':
        // "clear" the buffer before serving a request
        strcpy(_debugOutputBuffer, "");
        Serial.println("--- BEGIN of crash info ---");
        SaveCrash.crashToBuffer(_debugOutputBuffer);
        Serial.println(_debugOutputBuffer);
        Serial.println("--- END of crash info ---");
        break;
      default:
        Serial.printf("%c typed\n", inChar);
        break;
    }
  }
}

void handleRoot()
{
  server.send(200, "text/html", "<html><body>Hello from ESP<br><a href='/log'>Crash Log</a></body></html>");
}

void handleLog()
{
  // "clear" the buffer before serving a request
  strcpy(_debugOutputBuffer, "");

  SaveCrash.crashToBuffer(_debugOutputBuffer);

  server.send(200, "text/plain", _debugOutputBuffer);
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
