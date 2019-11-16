/*
  Example application to show how to save crash information
  to ESP8266's flash using EspSaveCrash library
  Please check repository below for details

  Repository: https://github.com/krzychb/EspSaveCrash
  File: SimpleCrash.ino
  Revision: 1.0.2
  Date: 19-Aug-2016
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

EspSaveCrash SaveCrash;

char *_debugOutputBuffer;

void setup(void)
{
  _debugOutputBuffer = (char *) calloc(2048, sizeof(char));

  Serial.begin(115200);
  Serial.println();
  Serial.println("SimpleCrash.ino");
  SaveCrash.print();

  Serial.println();
  Serial.println("Press a key + <enter>");
  Serial.println("0 : attempt to divide by zero");
  Serial.println("e : attempt to read through a pointer to no object");
  Serial.println("c : clear crash information");
  Serial.println("p : print crash information");
  Serial.println("b : store crash information to buffer and print buffer");
  Serial.println();
}


void loop(void)
{
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
        SaveCrash.crashToBuffer(_debugOutputBuffer);
        Serial.println("--- BEGIN of crash info from buffer ---");
        Serial.println(_debugOutputBuffer);
        Serial.println("--- END of crash info from buffer ---");
        break;
      default:
        Serial.printf("%c typed\n", inChar);
        break;
    }
  }
}

