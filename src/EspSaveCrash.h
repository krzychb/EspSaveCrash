/*
  This in an Arduino library to save exception details
  and stack trace to flash in case of ESP8266 crash.
  Please check repository below for details

  Repository: https://github.com/krzychb/EspSaveCrash
  File: EspSaveCrash.h
  Revision: 1.1.0
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

#ifndef _ESPSAVECRASH_H_
#define _ESPSAVECRASH_H_

#include "Arduino.h"
#include "EEPROM.h"
#include "user_interface.h"

/**
 * Layout of crash data saved to EEPROM (flash)
 *
 * 1. Crash counter / how many crashes are saved
 * 2. Next available space in EEPROM to write data
 * 3. Crash Data Set 1
 * 4. Crash Data Set 2
 * 5. ...
 */
#define SAVE_CRASH_COUNTER          0x00  // 1 byte
#define SAVE_CRASH_WRITE_FROM       0x01  // 2 bytes
#define SAVE_CRASH_DATA_SETS        0x03  // beginning of crash data sets
// Crash Data Set 1                       // variable length
// Crash Data Set 2                       // variable length
// ...                                    // variable length

/**
 * Structure of the single crash data set
 *
 *  1. Crash time
 *  2. Restart reason
 *  3. Exception cause
 *  4. epc1
 *  5. epc2
 *  6. epc3
 *  7. excvaddr
 *  8. depc
 *  9. address of stack start
 * 10. address of stack end
 * 11. stack trace bytes
 *     ...
 */
#define SAVE_CRASH_CRASH_TIME       0x00  // 4 bytes
#define SAVE_CRASH_RESTART_REASON   0x04  // 1 byte
#define SAVE_CRASH_EXCEPTION_CAUSE  0x05  // 1 byte
#define SAVE_CRASH_EPC1             0x06  // 4 bytes
#define SAVE_CRASH_EPC2             0x0A  // 4 bytes
#define SAVE_CRASH_EPC3             0x0E  // 4 bytes
#define SAVE_CRASH_EXCVADDR         0x12  // 4 bytes
#define SAVE_CRASH_DEPC             0x16  // 4 bytes
#define SAVE_CRASH_STACK_START      0x1A  // 4 bytes
#define SAVE_CRASH_STACK_END        0x1E  // 4 bytes
#define SAVE_CRASH_STACK_TRACE      0x22  // variable


class EspSaveCrash
{
  public:
    EspSaveCrash(uint16_t = 0x0010, uint16_t = 0x0200);
    void print(Print& outDevice = Serial);
    size_t print(char* userBuffer, size_t size);

    // deprecated, for backwards-compatibility only
    void crashToBuffer(char* userBuffer);

    void clear();
    int count();
    uint16_t offset();
    uint16_t size();

    //These have to be public in order to be accessed by callback
    static uint16_t _offset;
    static uint16_t _size;
    static bool _persistEEPROM;
  private:
    // none
};

//TODO: How to gracefully report to user with new static vars?
// check if configuration of EEPROM layout will fit into EEPROM sector size
//#if SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_SPACE_SIZE > SPI_FLASH_SEC_SIZE
//  #warning Check configuration of EEPROM layout!
//#endif

#endif //_ESPSAVECRASH_H_
