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

EspSaveCrash SaveCrash;

const char* ssid = "********";
const char* password = "********";

WiFiServer server(80);


void setup(void)
{
  Serial.begin(115200);
  Serial.println("\nRemoteCrashCheck.ino");

  SaveCrash.print();

  Serial.printf("Connecting to %s ", ssid);
  WiFi.persistent(false);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");

  server.begin();
  Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
  Serial.println("\nPress a key + <enter>");
  Serial.println("0 : attempt to divide by zero");
  Serial.println("e : attempt to read through a pointer to no object");
  Serial.println("c : clear crash information");
}


void loop(void)
{
  // read line by line what the client (web browser) is requesting
  WiFiClient client = server.available();
  if (client)
  {
    Serial.println("\n[Client connected]");
    while (client.connected())
    {
      // read line by line what the client (web browser) is requesting
      if (client.available())
      {
        String line = client.readStringUntil('\r');
        // look for the end of client's request, that is marked with an empty line
        if (line.length() == 1 && line[0] == '\n')
        {
          // send response header to the web browser
          client.print("HTTP/1.1 200 OK\r\n");
          client.print("Content-Type: text/plain\r\n");
          client.print("Connection: close\r\n");
          client.print("\r\n");
          // send crash information to the web browser
          SaveCrash.print(client);
          break;
        }
      }
    }
    delay(1); // give the web browser time to receive the data
    // close the connection:
    client.stop();
    Serial.println("[Client disonnected]");
  }

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
      default:
        Serial.printf("%c typed\n", inChar);
        break;
    }
  }
}
