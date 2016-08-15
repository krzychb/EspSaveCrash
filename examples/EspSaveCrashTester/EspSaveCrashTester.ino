/*
  Example application to show how to save crash information
  to ESP8266's flash using EspSaveCrash library
  Please check repository below for details

  Repository: https://github.com/krzychb/EspSaveCrash
  File: EspSaveCrashTester.ino
  Revision: 1.0.0
  Date: 15-Aug-2016
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


void setup(void)
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("EspSaveCrashTester.ino\n");

  SaveCrash.print();

  Serial.println("\n? - help");
}


int* nullPointer = NULL;

void loop(void)
{
  if (Serial.available() > 0)
  {
    char inChar = Serial.read();
    switch (inChar)
    {
      case ' ':
        SaveCrash.print();
        break;
      case 'c':
        SaveCrash.clear();
        Serial.println("Crash information cleared");
        break;
      case 's':
        Serial.printf("Crashing with software WDT (%ld ms) ...\n", millis());
        while (true)
        {
          // stay in an infinite loop doing nothing
          // this way other process can not be executed
        }
        break;
      case 'r':
        Serial.printf("Reset ESP (%ld ms) ...\n", millis());
        ESP.reset();
        break;
      case 't':
        Serial.printf("Restart ESP (%ld ms) ...\n", millis());
        ESP.restart();
        break;
      case 'h':
        Serial.printf("Crashing with hardware WDT (%ld ms) ...\n", millis());
        ESP.wdtDisable();
        while (true)
        {
          // stay in an infinite loop doing nothing
          // this way other process can not be executed
          //
          // Note:
          // Hardware wdt kicks in if software wdt is unable to perfrom
          // Nothing will be saved in EEPROM for the hardware wdt
        }
        break;
      case '0':
        Serial.printf("Crashing with 'division by zero' exeption (%ld ms) ...\n", millis());
        int result, zero;
        zero = 0;
        result = 1 / zero;
        Serial.printf("Result = %d\n", result);
        break;
      case 'e':
        Serial.printf("Crashing with 'read through a pointer to no object' exeption (%ld ms) ...\n", millis());
        // null pointer dereference - read
        // attempt to read a value through a null pointer
        Serial.print(*nullPointer);
        break;
      case 'x':
        Serial.printf("Crashing with 'write through a pointer to no object' exeption (%ld ms) ...\n", millis());
        // null pointer dereference - write
        // attempt to write a value through a null pointer
        *nullPointer = 0;
        break;
      case '?':
        Serial.println("Press a key + <enter>");
        Serial.println("<space> : print crash information");
        Serial.println("c : clear crash information");
        Serial.println("r/t : reset / restart module");
        Serial.println("? : print help");
        Serial.println("Crash this application with");
        Serial.println("s/h : software / harware WDT");
        Serial.println("0 : 'division by zero' exeption");
        Serial.println("e : 'read through a pointer to no object' exeption");
        Serial.println("x : 'write through a pointer to no object' exeption");
        break;
      default:
        Serial.printf("Case? (%c) / ? - help\n", inChar);
        break;
    }
  }
}
