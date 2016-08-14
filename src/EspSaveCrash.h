/*
  This in an Arduino library to save exception details
  and stack trace to flash in case of ESP8266 crash.
  Please check repository below for details

  Repository: https://github.com/krzychb/EspSaveCrash
  File: EspSaveCrash.h
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

#ifndef _ESPSAVECRASH_H_
#define _ESPSAVECRASH_H_

#include "Arduino.h"
#include "EEPROM.h"
#include "user_interface.h"

//
// Configuration of EEPROM layout
//
// Note that for using EEPROM we are also reserving a RAM buffer
// The buffer size will be bigger by SAVE_CRASH_EEPROM_OFFSET than what we actually need
// What we really need is SAVE_CRASH_SPACE_SIZE
//
#define SAVE_CRASH_EEPROM_OFFSET    0x0100  // adjust it to reserve space to store other data in EEPROM
#define SAVE_CRASH_SPACE_SIZE       0x0200  // space reserved to store crash data

//
// Crash data is saved to flash according to layout below
//
// How may crashes are saved to flash
#define SAVE_CRASH_COUNTER          0x00  // 1 byte
#define SAVE_CRASH_WRITE_FROM       0x01  // 2 bytes
#define SAVE_CRASH_DATA_SETS        0x03  // begining of crash data sets
// Crash Data Set 1                       // variable length
// Crash Data Set 2                       // variable length
// ...                                    // variable length

//
// Structure of Crash Data Set
//
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
    EspSaveCrash();
    void print();
    void clear();

  private:
    // none
};


extern EspSaveCrash SaveCrash;

// check if configuration of EEPROM layout will fit into EEPROM sector size
#if SAVE_CRASH_EEPROM_OFFSET + SAVE_CRASH_SPACE_SIZE > SPI_FLASH_SEC_SIZE
  #warning Check configuration of EEPROM layout!
#endif

#endif //_ESPSAVECRASH_H_
