/*
  This in an Arduino library to save exception details
  and stack trace to flash in case of ESP8266 crash.
  Please check repository below for details

  Repository: https://github.com/krzychb/EspSaveCrash
  File: EspSaveCrash.cpp
  Revision: 1.0.2
  Date: 18-Aug-2016
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

/**
 * EEPROM layout
 *
 * Note that for using EEPROM we are also reserving a RAM buffer
 * The buffer size will be bigger by static uint16_t _offset than what we actually need
 * The space that we really need is defined by static uint16_t _size
 */
uint16_t EspSaveCrash::_offset = 0x0010;
uint16_t EspSaveCrash::_size = 0x0200;

/**
 * Save crash information in EEPROM
 * This function is called automatically if ESP8266 suffers an exception
 * It should be kept quick / consise to be able to execute before hardware wdt may kick in
 */
extern "C" void custom_crash_callback(struct rst_info * rst_info, uint32_t stack, uint32_t stack_end )
{
  // Note that 'EEPROM.begin' method is reserving a RAM buffer
  // The buffer size is SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_SPACE_SIZE
  EEPROM.begin(EspSaveCrash::_offset + EspSaveCrash::_size);

  byte crashCounter = EEPROM.read(EspSaveCrash::_offset + SAVE_CRASH_COUNTER);
  int16_t writeFrom;
  if(crashCounter == 0)
  {
    writeFrom = SAVE_CRASH_DATA_SETS;
  }
  else
  {
    EEPROM.get(EspSaveCrash::_offset + SAVE_CRASH_WRITE_FROM, writeFrom);
  }

  // is there free EEPROM space avialable to save data for this crash?
  if (writeFrom + SAVE_CRASH_STACK_TRACE > EspSaveCrash::_size)
  {
    return;
  }

  // increment crash counter and write it to EEPROM
  EEPROM.write(EspSaveCrash::_offset + SAVE_CRASH_COUNTER, ++crashCounter);

  // now address EEPROM contents including _offset
  writeFrom += EspSaveCrash::_offset;

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
    if (currentAddress - EspSaveCrash::_offset > EspSaveCrash::_size)
    {
      // ToDo: flag an incomplete stack trace written to EEPROM!
      break;
    }
  }
  // now exclude _offset from address written to EEPROM
  currentAddress -= EspSaveCrash::_offset;
  EEPROM.put(EspSaveCrash::_offset + SAVE_CRASH_WRITE_FROM, currentAddress);

  EEPROM.commit();
}


/**
 * The class constructor
 */
EspSaveCrash::EspSaveCrash(uint16_t off, uint16_t size)
{
  _offset = off;
  _size = size;
}

/**
 * Clear crash information saved in EEPROM
 * In fact only crash counter is cleared
 * The crash data is not deleted
 */
void EspSaveCrash::clear(void)
{
  // Note that 'EEPROM.begin' method is reserving a RAM buffer
  // The buffer size is SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_SPACE_SIZE
  EEPROM.begin(_offset + _size);
  // clear the crash counter
  EEPROM.write(_offset + SAVE_CRASH_COUNTER, 0);
  EEPROM.end();
}


/**
 * Print out crash information that has been previusly saved in EEPROM
 * @param outputDev Print&    Optional. Where to print: Serial, Serial, WiFiClient, etc.
 */
void EspSaveCrash::print(Print& outputDev)
{
  // Note that 'EEPROM.begin' method is reserving a RAM buffer
  // The buffer size is SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_SPACE_SIZE
  EEPROM.begin(_offset + _size);
  byte crashCounter = EEPROM.read(_offset + SAVE_CRASH_COUNTER);
  if (crashCounter == 0)
  {
    outputDev.println("No crashes saved");
    return;
  }

  outputDev.println("Crash information recovered from EEPROM");
  int16_t readFrom = _offset + SAVE_CRASH_DATA_SETS;
  for (byte k = 0; k < crashCounter; k++)
  {
    uint32_t crashTime;
    EEPROM.get(readFrom + SAVE_CRASH_CRASH_TIME, crashTime);
    outputDev.printf("Crash # %d at %ld ms\n", k + 1, crashTime);

    outputDev.printf("Restart reason: %d\n", EEPROM.read(readFrom + SAVE_CRASH_RESTART_REASON));
    outputDev.printf("Exception cause: %d\n", EEPROM.read(readFrom + SAVE_CRASH_EXCEPTION_CAUSE));

    uint32_t epc1, epc2, epc3, excvaddr, depc;
    EEPROM.get(readFrom + SAVE_CRASH_EPC1, epc1);
    EEPROM.get(readFrom + SAVE_CRASH_EPC2, epc2);
    EEPROM.get(readFrom + SAVE_CRASH_EPC3, epc3);
    EEPROM.get(readFrom + SAVE_CRASH_EXCVADDR, excvaddr);
    EEPROM.get(readFrom + SAVE_CRASH_DEPC, depc);
    outputDev.printf("epc1=0x%08x epc2=0x%08x epc3=0x%08x excvaddr=0x%08x depc=0x%08x\n", epc1, epc2, epc3, excvaddr, depc);

    uint32_t stackStart, stackEnd;
    EEPROM.get(readFrom + SAVE_CRASH_STACK_START, stackStart);
    EEPROM.get(readFrom + SAVE_CRASH_STACK_END, stackEnd);
    outputDev.println(">>>stack>>>");
    int16_t currentAddress = readFrom + SAVE_CRASH_STACK_TRACE;
    int16_t stackLength = stackEnd - stackStart;
    uint32_t stackTrace;
    for (int16_t i = 0; i < stackLength; i += 0x10)
    {
      outputDev.printf("%08x: ", stackStart + i);
      for (byte j = 0; j < 4; j++)
      {
        EEPROM.get(currentAddress, stackTrace);
        outputDev.printf("%08x ", stackTrace);
        currentAddress += 4;
        if (currentAddress - _offset > _size)
        {
          outputDev.println("\nIncomplete stack trace saved!");
          goto eepromSpaceEnd;
        }
      }
      outputDev.println();
    }
    eepromSpaceEnd:
    outputDev.println("<<<stack<<<");
    readFrom = readFrom + SAVE_CRASH_STACK_TRACE + stackLength;
  }
  int16_t writeFrom;
  EEPROM.get(_offset + SAVE_CRASH_WRITE_FROM, writeFrom);
  EEPROM.end();

  // is there free EEPROM space avialable to save data for next crash?
  if (writeFrom + SAVE_CRASH_STACK_TRACE > _size)
  {
    outputDev.println("No more EEPROM space available to save crash information!");
  }
  else
  {
    outputDev.printf("EEPROM space available: 0x%04x bytes\n", _size - writeFrom);
  }
}

/**
 * @brief      Set crash information that has been previusly saved in EEPROM to user buffer
 * @param      userBuffer  The user buffer
 */
void EspSaveCrash::crashToBuffer(char* userBuffer)
{
  char *tmpBuffer = (char*)calloc(2048, sizeof(char));

  // Note that 'EEPROM.begin' method is reserving a RAM buffer
  // The buffer size is SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_SPACE_SIZE
  EEPROM.begin(_offset + _size);
  byte crashCounter = EEPROM.read(_offset + SAVE_CRASH_COUNTER);
  if (crashCounter == 0)
  {
    strcpy(userBuffer, "No crashes saved");
    return;
  }

  strcat(userBuffer, "Crash information recovered from EEPROM\n");

  int16_t readFrom = _offset + SAVE_CRASH_DATA_SETS;
  for (byte k = 0; k < crashCounter; k++)
  {
    uint32_t crashTime;
    EEPROM.get(readFrom + SAVE_CRASH_CRASH_TIME, crashTime);
    sprintf(tmpBuffer, "Crash # %d at %ld ms\n", k + 1, crashTime);
    strcat(userBuffer, tmpBuffer);

    sprintf(tmpBuffer, "Restart reason: %d\n", EEPROM.read(readFrom + SAVE_CRASH_RESTART_REASON));
    strcat(userBuffer, tmpBuffer);

    sprintf(tmpBuffer, "Exception cause: %d\n", EEPROM.read(readFrom + SAVE_CRASH_EXCEPTION_CAUSE));
    strcat(userBuffer, tmpBuffer);

    uint32_t epc1, epc2, epc3, excvaddr, depc;
    EEPROM.get(readFrom + SAVE_CRASH_EPC1, epc1);
    EEPROM.get(readFrom + SAVE_CRASH_EPC2, epc2);
    EEPROM.get(readFrom + SAVE_CRASH_EPC3, epc3);
    EEPROM.get(readFrom + SAVE_CRASH_EXCVADDR, excvaddr);
    EEPROM.get(readFrom + SAVE_CRASH_DEPC, depc);
    sprintf(tmpBuffer, "epc1=0x%08x epc2=0x%08x epc3=0x%08x excvaddr=0x%08x depc=0x%08x\n", epc1, epc2, epc3, excvaddr, depc);
    strcat(userBuffer, tmpBuffer);

    uint32_t stackStart, stackEnd;
    EEPROM.get(readFrom + SAVE_CRASH_STACK_START, stackStart);
    EEPROM.get(readFrom + SAVE_CRASH_STACK_END, stackEnd);
    sprintf(tmpBuffer, ">>>stack>>>\n");
    strcat(userBuffer, tmpBuffer);

    int16_t currentAddress = readFrom + SAVE_CRASH_STACK_TRACE;
    int16_t stackLength = stackEnd - stackStart;
    uint32_t stackTrace;
    for (int16_t i = 0; i < stackLength; i += 0x10)
    {
      sprintf(tmpBuffer, "%08x: ", stackStart + i);
      strcat(userBuffer, tmpBuffer);

      for (byte j = 0; j < 4; j++)
      {
        EEPROM.get(currentAddress, stackTrace);
        sprintf(tmpBuffer, "%08x ", stackTrace);
        strcat(userBuffer, tmpBuffer);

        currentAddress += 4;
        if (currentAddress - _offset > _size)
        {
          sprintf(tmpBuffer, "\nIncomplete stack trace saved!\n");
          strcat(userBuffer, tmpBuffer);

          goto eepromSpaceEnd;
        }
      }
      strcat(userBuffer, "\n");

    }
    eepromSpaceEnd:
    strcat(userBuffer, "<<<stack<<<\n");

    readFrom = readFrom + SAVE_CRASH_STACK_TRACE + stackLength;
  }
  int16_t writeFrom;
  EEPROM.get(_offset + SAVE_CRASH_WRITE_FROM, writeFrom);
  EEPROM.end();

  // is there free EEPROM space avialable to save data for next crash?
  if (writeFrom + SAVE_CRASH_STACK_TRACE > _size)
  {
    strcat(userBuffer, "No more EEPROM space available to save crash information!\n");
  }
  else
  {
    sprintf(tmpBuffer, "EEPROM space available: 0x%04x bytes\n", _size - writeFrom);
    strcat(userBuffer, tmpBuffer);
  }

  free(tmpBuffer);
}

/**
 * Get the count of crash data sets saved in EEPROM
 */
int EspSaveCrash::count()
{
  EEPROM.begin(_offset + _size);
  int crashCounter = EEPROM.read(_offset + SAVE_CRASH_COUNTER);
  EEPROM.end();
  return crashCounter;
}

/**
 * Get the memory offset value
 */
uint16_t EspSaveCrash::offset()
{
  return _offset;
}

/**
 * Get the memory size value
 */
uint16_t EspSaveCrash::size()
{
  return _size;
}