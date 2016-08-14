/*
  This in an Arduino library to save exception details
  and stack trace to flash in case of ESP8266 crash.
  Please check repository below for details

  Repository: https://github.com/krzychb/EspSaveCrash
  File: EspSaveCrash.cpp
  Revision: 1.0.0
  Date: 14-Aug-2016
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

extern "C" void custom_crash_callback(struct rst_info * rst_info, uint32_t stack, uint32_t stack_end )
{

  // Note that 'EEPROM.begin' method is reserving a RAM buffer
  // The buffer size is SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_SPACE_SIZE
  EEPROM.begin(SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_SPACE_SIZE);

  byte crashCounter = EEPROM.read(SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_COUNTER);
  int16_t writeFrom;
  if(crashCounter == 0)
  {
    writeFrom = SAVE_CRASH_DATA_SETS;
  }
  else
  {
    EEPROM.get(SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_WRITE_FROM, writeFrom);
  }

  // is there free EEPROM space avialable to save data for this crash?
  if (writeFrom + SAVE_CRASH_STACK_TRACE > SAVE_CRASH_SPACE_SIZE)
  {
    return;
  }

  // increment crash counter and write it to EEPROM
  EEPROM.write(SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_COUNTER, ++crashCounter);

  // now address EEPROM contents including SAVE_CRASH_EEPROM_OFFSET
  writeFrom += SAVE_CRASH_EEPROM_OFFSET;

  // write crash time to EEPROM
  uint32_t crashTime = millis();
  EEPROM.put(writeFrom + SAVE_CRASH_CRASH_TIME, crashTime);

  // write reset info to EEPROM
  EEPROM.write(writeFrom + SAVE_CRASH_RESTART_REASON, rst_info->reason);
  EEPROM.write(writeFrom + SAVE_CRASH_EXCEPTION_CAUSE, rst_info->exccause);

  // write epc1, epc2, epc3, excvaddr and depc to EEPROM
  EEPROM.put(writeFrom + SAVE_CRASH_EPC1, rst_info->epc1);
  EEPROM.put(writeFrom + SAVE_CRASH_EPC2, rst_info->epc2);
  EEPROM.put(writeFrom + SAVE_CRASH_EPC3, rst_info->epc3);
  EEPROM.put(writeFrom + SAVE_CRASH_EXCVADDR, rst_info->excvaddr);
  EEPROM.put(writeFrom + SAVE_CRASH_DEPC, rst_info->depc);

  // write stack start and end address to EEPROM
  EEPROM.put(writeFrom + SAVE_CRASH_STACK_START, stack);
  EEPROM.put(writeFrom + SAVE_CRASH_STACK_END, stack_end);

  // write stack trace to EEPROM
  int16_t currentAddress = writeFrom + SAVE_CRASH_STACK_TRACE;
  for (uint32_t iAddress = stack; iAddress < stack_end; iAddress++)
  {
    byte* byteValue = (byte*) iAddress;
    EEPROM.write(currentAddress++, *byteValue);
    if (currentAddress - SAVE_CRASH_EEPROM_OFFSET > SAVE_CRASH_SPACE_SIZE)
    {
      // ToDo: flag an incomplete stack trace written to EEPROM!
      break;
    }
  }
  // now exclude SAVE_CRASH_EEPROM_OFFSET from address written to EEPROM
  currentAddress -= SAVE_CRASH_EEPROM_OFFSET;
  EEPROM.put(SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_WRITE_FROM, currentAddress);

  EEPROM.commit();
}


EspSaveCrash::EspSaveCrash(void) {}


void EspSaveCrash::clear(void)
{
  // Note that 'EEPROM.begin' method is reserving a RAM buffer
  // The buffer size is SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_SPACE_SIZE
  EEPROM.begin(SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_SPACE_SIZE);
  // clear the crash counter
  EEPROM.write(SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_COUNTER, 0);
  EEPROM.end();
}


void EspSaveCrash::print(void)
{
  // Note that 'EEPROM.begin' method is reserving a RAM buffer
  // The buffer size is SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_SPACE_SIZE
  EEPROM.begin(SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_SPACE_SIZE);

  byte crashCounter = EEPROM.read(SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_COUNTER);
  if (crashCounter == 0)
  {
    Serial.println("No any crashes saved");
    return;
  }

  Serial.println("Crash information recovered from EEPROM");
  int16_t readFrom = SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_DATA_SETS;
  for (byte k = 0; k < crashCounter; k++)
  {
    uint32_t crashTime;
    EEPROM.get(readFrom + SAVE_CRASH_CRASH_TIME, crashTime);
    Serial.printf("Crash # %d at %d ms\n", k + 1, crashTime);

    Serial.printf("Reason of restart: %d\n", EEPROM.read(readFrom + SAVE_CRASH_RESTART_REASON));
    Serial.printf("Exception cause: %d\n", EEPROM.read(readFrom + SAVE_CRASH_EXCEPTION_CAUSE));

    uint32_t epc1, epc2, epc3, excvaddr, depc;
    EEPROM.get(readFrom + SAVE_CRASH_EPC1, epc1);
    EEPROM.get(readFrom + SAVE_CRASH_EPC2, epc2);
    EEPROM.get(readFrom + SAVE_CRASH_EPC3, epc3);
    EEPROM.get(readFrom + SAVE_CRASH_EXCVADDR, excvaddr);
    EEPROM.get(readFrom + SAVE_CRASH_DEPC, depc);
    Serial.printf("epc1=0x%08x epc2=0x%08x epc3=0x%08x excvaddr=0x%08x depc=0x%08x\n", epc1, epc2, epc3, excvaddr, depc);

    uint32_t stackStart, stackEnd;
    EEPROM.get(readFrom + SAVE_CRASH_STACK_START, stackStart);
    EEPROM.get(readFrom + SAVE_CRASH_STACK_END, stackEnd);
    Serial.println(">>>stack>>>");
    int16_t currentAddress = readFrom + SAVE_CRASH_STACK_TRACE;
    int16_t stackLength = stackEnd - stackStart;
    uint32_t stackTrace;
    for (int16_t i = 0; i < stackLength; i += 0x10)
    {
      Serial.printf("%08x: ", stackStart + i);
      for (byte j = 0; j < 4; j++)
      {
        EEPROM.get(currentAddress, stackTrace);
        Serial.printf("%08x ", stackTrace);
        currentAddress += 4;
        if (currentAddress - SAVE_CRASH_EEPROM_OFFSET > SAVE_CRASH_SPACE_SIZE)
        {
          Serial.println("\nIncomplete stack trace saved!");
          break;
        }
      }
      Serial.println();
    }
    Serial.println("<<<stack<<<");
    readFrom = readFrom + SAVE_CRASH_STACK_TRACE + stackLength;
  }
  int16_t writeFrom;
  EEPROM.get(SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_WRITE_FROM, writeFrom);
  EEPROM.end();

  // is there free EEPROM space avialable to save data for next crash?
  if (writeFrom + SAVE_CRASH_STACK_TRACE > SAVE_CRASH_SPACE_SIZE)
  {
    Serial.println("No more EEPROM space available to save crash information!");
  }
  else
  {
    Serial.printf("EEPROM space available: 0x%04x bytes\n", SAVE_CRASH_SPACE_SIZE - writeFrom);
  }
}


EspSaveCrash SaveCrash;
